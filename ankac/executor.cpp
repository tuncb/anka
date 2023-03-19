#include "executor.h"

#include <numeric>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>

#include "internal_functions.h"

typedef std::vector<int> (*IntToIntVectorFunc)(int);
typedef int (*IntToIntFunc)(int);
typedef int (*IntVectorToIntFunc)(const std::vector<int> &);
typedef std::vector<int> (*IntVectorToIntArrayFunc)(const std::vector<int> &);

auto findFunction(const std::string &name) -> std::optional<anka::InternalFunction>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
  {
    return iter->second;
  }
  return std::nullopt;
}

auto foldIntToIntArrayFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  if (input.type != anka::WordType::IntegerNumber)
    return std::nullopt;

  auto funcptr = (IntToIntVectorFunc)func.ptr;
  auto output = funcptr(context.integerNumbers[input.index]);

  return anka::createWord(context, std::move(output));
}

auto foldIntToIntFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  auto funcptr = (IntToIntFunc)func.ptr;

  if (input.type == anka::WordType::IntegerNumber)
  {
    auto value = funcptr(context.integerNumbers[input.index]);
    return anka::createWord(context, value);
  }

  if (input.type == anka::WordType::IntegerArray)
  {
    const auto& vec = context.integerArrays[input.index];
    std::vector<int> output;
    output.reserve(vec.size());
    for (auto inp: vec)
    {
      output.push_back(funcptr(inp));
    }
    return anka::createWord(context, std::move(output));
  }
  return std::nullopt;
}

auto foldIntArrayToIntFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  if (input.type != anka::WordType::IntegerArray)
    return std::nullopt;

  auto funcptr = (IntVectorToIntFunc)func.ptr;
  auto value = funcptr(context.integerArrays[input.index]);

  context.integerNumbers.push_back(value);
  return anka::Word{anka::WordType::IntegerNumber, context.integerNumbers.size() - 1};
}

auto foldIntArrayToIntArrayFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  if (input.type != anka::WordType::IntegerArray)
    return std::nullopt;

  auto funcptr = (IntVectorToIntArrayFunc)func.ptr;
  auto output = funcptr(context.integerArrays[input.index]);

  return anka::createWord(context, std::move(output));
}

auto foldFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  using namespace anka;
  switch (func.type)
  {
  case InternalFunctionType::IntToIntArray:
    return foldIntToIntArrayFunction(context, input, func);
  case InternalFunctionType::IntToInt:
    return foldIntToIntFunction(context, input, func);
  case InternalFunctionType::IntArrayToInt:
    return foldIntArrayToIntFunction(context, input, func);
  case InternalFunctionType::IntArrayToIntArray:
    return foldIntArrayToIntArrayFunction(context, input, func);
  }
  return std::nullopt;
}

auto fold(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  using namespace anka;
  if (w1.type == WordType::Name)
  {
    auto name = context.names[w1.index];
    if (findFunction(name))
    {
      throw anka::ExecutionError{w1, w2, "Could not fold words."};
    }
    throw anka::ExecutionError{w1, std::nullopt, "Could not find name."};
  }
  if (w2.type == WordType::Name)
  {
    auto name = context.names[w2.index];
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

  auto name = context.names[word.index];
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

    auto init = sentence.words.back();
    auto rv = sentence.words | views::reverse | views::drop(1);

    wordOpt = std::accumulate(rv.begin(), rv.end(), init,
                              [&ast](const Word &w1, const Word &w2) { return fold(ast.context, w1, w2); });

    if (wordOpt.has_value())
      wordOpt = expandName(ast.context, wordOpt.value());
  }

  return wordOpt;
}
