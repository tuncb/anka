#include "executor.h"

#include <format>
#include <numeric>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>

#include "internal_functions.h"

// forward declerations
auto executeWords(anka::Context &context, const std::vector<anka::Word> &words) -> std::optional<anka::Word>;

auto nameExists(const std::string &name) -> bool
{
  const auto &internalFunctions = anka::getInternalFunctions();
  auto iter = internalFunctions.find(name);
  return iter != internalFunctions.end();
}

auto isDoubleType(const anka::Word &word) -> bool
{
  return word.type == anka::WordType::DoubleNumber || word.type == anka::WordType::DoubleArray;
}

auto isIntegerType(const anka::Word &word) -> bool
{
  return word.type == anka::WordType::IntegerNumber || word.type == anka::WordType::IntegerArray;
}

auto checkOverloadCompatibility(const anka::Context &context, anka::InternalFunctionType type, const anka::Word &word)
    -> bool
{
  using namespace anka;

  auto first = anka::getWord(context, word, 0);
  if (!first.has_value())
    return false;

  switch (type)
  {
  case InternalFunctionType::Int__IntArray:
  case InternalFunctionType::Int__Int:
  case InternalFunctionType::Int__Double:
    return first.value().type == WordType::IntegerNumber || first.value().type == WordType::IntegerArray;
  case InternalFunctionType::IntArray__Int:
  case InternalFunctionType::IntArray__IntArray:
    return first.value().type == WordType::IntegerArray;
  case InternalFunctionType::Double__Double:
    return first.value().type == WordType::DoubleNumber || first.value().type == WordType::DoubleArray;
  case InternalFunctionType::DoubleArray__Int:
  case InternalFunctionType::DoubleArray__Double:
  case InternalFunctionType::DoubleArray__DoubleArray:
    return first.value().type == WordType::DoubleArray;
  case InternalFunctionType::Bool__Bool:
    return first.value().type == WordType::Boolean || first.value().type == WordType::BooleanArray;
  case InternalFunctionType::BoolArray__Int:
  case InternalFunctionType::BoolArray__Bool:
  case InternalFunctionType::BoolArray__BoolArray:
    return first.value().type == WordType::BooleanArray;
  };

  auto second = anka::getWord(context, word, 1);
  if (!second.has_value())
    return false;

  switch (type)
  {
  case InternalFunctionType::Int_Int__Int:
  case InternalFunctionType::Int_Int_Bool:
    return (first.value().type == WordType::IntegerNumber || first.value().type == WordType::IntegerArray) &&
           (second.value().type == WordType::IntegerNumber || second.value().type == WordType::IntegerArray);
  case InternalFunctionType::Bool_Bool__Bool:
    return (first.value().type == WordType::Boolean || first.value().type == WordType::BooleanArray) &&
           (second.value().type == WordType::Boolean || second.value().type == WordType::BooleanArray);
  case InternalFunctionType::Double_Double__Double:
  case InternalFunctionType::Double_Double_Bool: {
    if (isDoubleType(first.value()) && isDoubleType(second.value()))
      return true;
    // Automatic int to double conversions
    return (isIntegerType(first.value()) && isDoubleType(second.value())) ||
           (isIntegerType(second.value()) && isDoubleType(first.value()));
  }
  };

  return false;
}

auto findOverload(const anka::Context &context, const std::string &name, const anka::Word &word)
    -> std::optional<anka::InternalFunction>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
  {
    for (auto &&overload : iter->second)
    {
      if (checkOverloadCompatibility(context, overload.type, word))
        return overload;
    }
  }
  return std::nullopt;
}

template <typename R, typename T1, typename T2, typename FuncType = R (*)(T1, T2)>
auto foldTwoArgumentWithRankPolyFunction(anka::Context &context, const anka::Word &input,
                                         const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  if (anka::getWordCount(context, input) != 2)
    return std::nullopt;

  auto funcptr = (FuncType)func.ptr;

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
  if (anka::getWordCount(context, input) != 1)
    return std::nullopt;

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
  if (anka::getWordCount(context, input) != 1)
    return std::nullopt;

  const auto &valOpt = extractValue<T>(context, input, 0);
  if (!valOpt)
    return std::nullopt;

  typedef R (*RealFuncType)(T);

  auto funcptr = (RealFuncType)func.ptr;
  auto output = funcptr(valOpt.value());
  return anka::createWord(context, std::move(output));
}

template <typename ReturnType>
auto foldDouble_Double__RVariations(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  typedef ReturnType (*FuncType)(double, double);

  if (anka::getWordCount(context, input) != 2)
    return std::nullopt;

  auto first = anka::getWord(context, input, 0).value();
  auto second = anka::getWord(context, input, 1).value();

  if (isDoubleType(first) && isDoubleType(second))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, double, double>(context, input, func);

  if (isDoubleType(first) && isIntegerType(second))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, double, int, FuncType>(context, input, func);

  if (isDoubleType(second) && isIntegerType(first))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, int, double, FuncType>(context, input, func);

  return std::nullopt;
}

auto foldFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  using namespace anka;
  switch (func.type)
  {
  case InternalFunctionType::Int__IntArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<int>, int>(context, input, func);
  case InternalFunctionType::Int__Int:
    return foldSingleArgumentWithRankPolyFunction<int, int>(context, input, func);
  case InternalFunctionType::Int__Double:
    return foldSingleArgumentWithRankPolyFunction<double, int>(context, input, func);
  case InternalFunctionType::Int_Int_Bool:
    return foldTwoArgumentWithRankPolyFunction<bool, int, int>(context, input, func);
  case InternalFunctionType::Int_Int__Int:
    return foldTwoArgumentWithRankPolyFunction<int, int, int>(context, input, func);
  case InternalFunctionType::IntArray__IntArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<int>, const std::vector<int> &>(context, input, func);
  case InternalFunctionType::IntArray__Int:
    return foldSingleArgumentNoRankPolyFunction<int, const std::vector<int> &>(context, input, func);
  case InternalFunctionType::Bool__Bool:
    return foldSingleArgumentWithRankPolyFunction<bool, bool>(context, input, func);
  case InternalFunctionType::BoolArray__Int:
    return foldSingleArgumentNoRankPolyFunction<int, const std::vector<bool> &>(context, input, func);
  case InternalFunctionType::BoolArray__Bool:
    return foldSingleArgumentNoRankPolyFunction<bool, const std::vector<bool> &>(context, input, func);
  case InternalFunctionType::Bool_Bool__Bool:
    return foldTwoArgumentWithRankPolyFunction<bool, bool, bool>(context, input, func);
  case InternalFunctionType::BoolArray__BoolArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<bool>, const std::vector<bool> &>(context, input, func);
  case InternalFunctionType::Double__Double:
    return foldSingleArgumentWithRankPolyFunction<double, double>(context, input, func);
  case InternalFunctionType::Double_Double__Double:
    return foldDouble_Double__RVariations<double>(context, input, func);
  case InternalFunctionType::Double_Double_Bool:
    return foldDouble_Double__RVariations<bool>(context, input, func);
  case InternalFunctionType::DoubleArray__Int:
    return foldSingleArgumentNoRankPolyFunction<int, const std::vector<double> &>(context, input, func);
  case InternalFunctionType::DoubleArray__Double:
    return foldSingleArgumentNoRankPolyFunction<double, const std::vector<double> &>(context, input, func);
  case InternalFunctionType::DoubleArray__DoubleArray:
    return foldSingleArgumentNoRankPolyFunction<std::vector<double>, const std::vector<double> &>(context, input, func);
  }
  return std::nullopt;
}

auto foldPlaceholder(anka::Context &context, const anka::Word &placeholder, const anka::Word &rhs) -> anka::Word
{
  using namespace anka;

  if (rhs.type == WordType::PlaceHolder)
    throw anka::ExecutionError{placeholder, rhs, "Cannot fold a placeholder to another."};

  if (rhs.type != WordType::Tuple)
  {
    if (placeholder.index == 1)
      return rhs;
    throw anka::ExecutionError{placeholder, rhs, std::format("Placeholder _{} is out of range.", placeholder.index)};
  }

  auto &&tup = getValue<const Tuple &>(context, rhs.index);
  auto idx = placeholder.index - 1;
  if (idx >= 0 && idx < tup.words.size())
  {
    return tup.words[idx];
  }

  throw anka::ExecutionError{placeholder, rhs, std::format("Placeholder _{} is out of range.", placeholder.index)};
}

auto foldtuple(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  auto &&tup = anka::getValue<const anka::Tuple &>(context, w2.index);

  using namespace anka;
  std::vector<Word> words;
  auto wordsToBeInserted = anka::getAllWords(context, w1);

  for (auto &&w : tup.words)
  {
    if (w.type == WordType::PlaceHolder)
    {
      if (w.index == 0)
      {
        words.insert(words.end(), wordsToBeInserted.begin(), wordsToBeInserted.end());
      }
      else
      {
        auto idx = w.index - 1;
        if (idx < 0 || idx >= wordsToBeInserted.size())
          throw anka::ExecutionError{w1, std::nullopt, std::format("Placeholder {} is out of range.", w.index)};
        words.push_back(wordsToBeInserted[idx]);
      }
    }
    else
    {
      words.push_back(w);
    }
  }
  return anka::createWord(context, {std::move(words), false});
}

auto createExecutorBlocks(anka::Context &context, const std::vector<anka::Word> &executorWords, const anka::Word &rhs)
    -> std::vector<std::vector<anka::Word>>
{
  using namespace anka;

  std::vector<std::vector<anka::Word>> sentences;
  for (auto &&lhs : executorWords)
  {
    if (lhs.type == WordType::Tuple)
    {
      auto &&tup = getValue<const Tuple &>(context, lhs.index);
      if (tup.isConnected)
      {
        sentences.back().push_back(lhs);
      }
      else
      {
        sentences.push_back({lhs});
      }
    }
    else
    {
      sentences.push_back({lhs});
    }
  }
  for (auto &&sentence : sentences)
  {
    sentence.push_back(rhs);
  }
  return sentences;
}

auto foldExecutor(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  using namespace anka;
  auto &&blocks = createExecutorBlocks(context, anka::getValue<const anka::Executor &>(context, w2.index).words, w1);

  std::vector<Word> res;
  for (auto &&block : blocks)
  {
    res.push_back(executeWords(context, block).value());
  }
  return createWord(context, Tuple{res, false});
}

auto fold(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  using namespace anka;
  // rhs
  if (w1.type == WordType::Name)
  {
    const auto &name = context.names[w1.index];
    if (nameExists(name))
    {
      throw anka::ExecutionError{w1, w2, "Could not fold words."};
    }
    throw anka::ExecutionError{w1, std::nullopt, "Could not find name."};
  }
  else if (w1.type == WordType::Block)
  {
    auto &&block = getValue<const Block &>(context, w1.index);
    auto words = std::vector<Word>{w2};
    words.insert(words.end(), block.words.begin(), block.words.end());
    return executeWords(context, words).value();
  }
  // lhs
  if (w2.type == WordType::Name)
  {
    const auto &name = context.names[w2.index];
    auto functOpt = findOverload(context, name, w1);
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
  else if (w2.type == WordType::Tuple)
  {
    return foldtuple(context, w1, w2);
  }
  else if (w2.type == WordType::PlaceHolder)
  {
    return foldPlaceholder(context, w2, w1);
  }
  else if (w2.type == WordType::Executor)
  {
    return foldExecutor(context, w1, w2);
  }
  else if (w2.type == WordType::Block)
  {
    auto &&block = getValue<const Block &>(context, w2.index);
    auto words = block.words;
    words.push_back(w1);
    return executeWords(context, words).value();
  }

  throw anka::ExecutionError{w1, w2, "Could not fold words."};
}

auto validateValue(anka::Context &context, const anka::Word &word) -> void
{
  if (word.type == anka::WordType::PlaceHolder)
    throw anka::ExecutionError{word, std::nullopt, "Could not find word indicated by the placeholder."};

  if (word.type != anka::WordType::Name)
    return;

  const auto &name = context.names[word.index];
  if (!nameExists(name))
  {
    throw anka::ExecutionError{word, std::nullopt, "Could not find word."};
  }
}

auto executeWords(anka::Context &context, const std::vector<anka::Word> &words) -> std::optional<anka::Word>
{
  using namespace ranges;
  using namespace anka;

  if (words.empty())
    return std::nullopt;

  const auto &init = words.back();
  auto rv = words | views::reverse | views::drop(1);

  return std::accumulate(rv.begin(), rv.end(), init,
                         [&context](const Word &w1, const Word &w2) { return fold(context, w1, w2); });
}

auto anka::execute(AST &ast) -> std::optional<Word>
{
  if (ast.sentences.empty())
    return std::nullopt;

  std::optional<Word> wordOpt = std::nullopt;
  for (const auto &sentence : ast.sentences)
  {
    if (sentence.words.empty())
      continue;

    wordOpt = executeWords(ast.context, sentence.words);
    if (wordOpt.has_value())
      validateValue(ast.context, wordOpt.value());
  }

  return wordOpt;
}
