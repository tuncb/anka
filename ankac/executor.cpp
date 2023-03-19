#include "executor.h"

#include <numeric>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>

#include "internal_functions.h"

auto findFunction(const std::string &name) -> std::optional<anka::InternalFunction>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
  {
    return iter->second;
  }
  return std::nullopt;
}

template <typename R, typename T1, typename T2>
auto foldTwoArgumentWithRankPolyFunction(anka::Context &context, const anka::Word &input,
                                         const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  typedef R (*RealFuncType)(T1, T2);
  auto funcptr = (RealFuncType)func.ptr;

  auto opt1 = extractValue<T1>(context, input, 0);
  auto opt2 = extractValue<T2>(context, input, 1);

  if (opt1 && opt2)
  {
    auto value = funcptr(opt1.value(), opt2.value());
    return anka::createWord(context, value);
  }

  auto optVec1 = extractValue<const std::vector<T1> &>(context, input, 0);
  auto optVec2 = extractValue<const std::vector<T2> &>(context, input, 1);

  if (optVec1 && optVec2)
  {
    auto &&vec1 = optVec1.value();
    auto &&vec2 = optVec2.value();

    if (vec1.size() != vec2.size())
      return std::nullopt;

    std::vector<R> output;
    output.reserve(vec1.size());

    for (size_t i = 0; i < vec1.size(); ++i)
    {
      output.emplace_back(funcptr(vec1[i], vec2[i]));
    }
    return anka::createWord(context, std::move(output));
  }

  if (opt1 && optVec2)
  {
    auto val1 = opt1.value();
    auto &&vec2 = optVec2.value();

    std::vector<R> output;
    output.reserve(vec2.size());

    for (size_t i = 0; i < vec2.size(); ++i)
    {
      output.emplace_back(funcptr(val1, vec2[i]));
    }
    return anka::createWord(context, std::move(output));
  }

  if (opt2 && optVec1)
  {
    auto val2 = opt2.value();
    auto &&vec1 = optVec1.value();

    std::vector<R> output;
    output.reserve(vec1.size());

    for (size_t i = 0; i < vec1.size(); ++i)
    {
      output.emplace_back(funcptr(vec1[i], val2));
    }
    return anka::createWord(context, std::move(output));
  }

  return std::nullopt;
}

template <typename R, typename T>
auto foldSingleArgumentWithRankPolyFunction(anka::Context &context, const anka::Word &input,
                                            const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  typedef R (*RealFuncType)(T);
  auto funcptr = (RealFuncType)func.ptr;

  auto opt1 = extractValue<T>(context, input, 0);

  if (opt1)
  {
    auto value = funcptr(opt1.value());
    return anka::createWord(context, value);
  }

  auto optVec1 = extractValue<const std::vector<T> &>(context, input, 0);
  if (optVec1)
  {
    auto &&vec1 = optVec1.value();
    std::vector<R> output;
    output.reserve(vec1.size());

    for (size_t i = 0; i < vec1.size(); ++i)
    {
      output.emplace_back(funcptr(vec1[i]));
    }
    return anka::createWord(context, std::move(output));
  }
  return std::nullopt;
}

template <typename R, typename T>
auto foldSingleArgumentNoRankPolyFunction(anka::Context &context, const anka::Word &input,
                                          const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  const auto &valOpt = extractValue<T>(context, input, 0);
  if (!valOpt)
    return std::nullopt;

  typedef R (*RealFuncType)(T);

  auto funcptr = (RealFuncType)func.ptr;
  auto output = funcptr(valOpt.value());
  return anka::createWord(context, std::move(output));
}

auto foldFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  using namespace anka;
  switch (func.type)
  {
  case InternalFunctionType::IntToIntArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<int>, int>(context, input, func);
  case InternalFunctionType::IntToInt:
    return foldSingleArgumentWithRankPolyFunction<int, int>(context, input, func);
  case InternalFunctionType::IntArrayToInt:
    return foldSingleArgumentNoRankPolyFunction<int, const std::vector<int> &>(context, input, func);
  case InternalFunctionType::IntArrayToIntArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<int>, const std::vector<int> &>(context, input, func);
  case InternalFunctionType::IntIntToInt:
    return foldTwoArgumentWithRankPolyFunction<int, int, int>(context, input, func);
  }
  return std::nullopt;
}

auto fold(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  using namespace anka;
  if (w1.type == WordType::Name)
  {
    const auto &name = context.names[w1.index];
    if (findFunction(name))
    {
      throw anka::ExecutionError{w1, w2, "Could not fold words."};
    }
    throw anka::ExecutionError{w1, std::nullopt, "Could not find name."};
  }
  if (w2.type == WordType::Name)
  {
    const auto &name = context.names[w2.index];
    auto functOpt = findFunction(name);
    if (functOpt)
    {
      auto wordOpt = foldFunction(context, w1, functOpt.value());
      if (!wordOpt)
      {
        throw anka::ExecutionError{w1, w2, "Could not fold words."};
      }
      return wordOpt.value();
    }
    throw anka::ExecutionError{std::nullopt, w2, "Could not find name."};
  }

  throw anka::ExecutionError{w1, w2, "Could not fold words."};
}

auto expandName(anka::Context &context, const anka::Word &word) -> anka::Word
{
  if (word.type != anka::WordType::Name)
    return word;

  const auto &name = context.names[word.index];
  if (!findFunction(name))
  {
    throw anka::ExecutionError{word, std::nullopt, "Could not find word."};
  }

  return word;
}

auto anka::execute(AST &ast) -> std::optional<Word>
{
  using namespace ranges;

  if (ast.sentences.empty())
    return std::nullopt;

  std::optional<Word> wordOpt = std::nullopt;
  for (const auto &sentence : ast.sentences)
  {
    if (sentence.words.empty())
      continue;

    const auto &init = sentence.words.back();
    auto rv = sentence.words | views::reverse | views::drop(1);

    wordOpt = std::accumulate(rv.begin(), rv.end(), init,
                              [&ast](const Word &w1, const Word &w2) { return fold(ast.context, w1, w2); });

    if (wordOpt.has_value())
      wordOpt = expandName(ast.context, wordOpt.value());
  }

  return wordOpt;
}
