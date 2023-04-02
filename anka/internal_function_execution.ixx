module;
#include "ast.h"

export module anka:internal_function_execution;

import :internal_functions;

namespace anka
{

auto isDoubleType(const anka::Word &word) -> bool
{
  return word.type == anka::WordType::DoubleNumber || word.type == anka::WordType::DoubleArray;
}

auto isIntegerType(const anka::Word &word) -> bool
{
  return word.type == anka::WordType::IntegerNumber || word.type == anka::WordType::IntegerArray;
}

auto isBooleanType(const anka::Word &word) -> bool
{
  return word.type == anka::WordType::Boolean || word.type == anka::WordType::BooleanArray;
}

auto findFunctionWithCategory(const anka::Context &context, const anka::Word &word, InternalFunctionCategory category)
    -> std::optional<anka::InternalFunction>
{
  if (word.type != WordType::Name)
    return std::nullopt;

  const auto &name = getValue<std::string>(context, word.index);
  auto &&internalFunctions = getInternalFunctions();
  if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
  {
    auto overloadIter = std::ranges::find_if(
        iter->second, [category](const anka::InternalFunction &fun) { return getCategory(fun.type) == category; });
    if (overloadIter != iter->second.end())
      return *overloadIter;
  }
  return std::nullopt;
}

auto hasFunctionCategory(const anka::Context &context, const anka::Word &word, InternalFunctionCategory category)
    -> bool
{
  auto opt = findFunctionWithCategory(context, word, category);
  return opt.has_value();
}

export auto checkOverloadCompatibility(const anka::Context &context, anka::InternalFunctionType type,
                                       const anka::Word &word) -> bool
{
  using namespace anka;

  auto firstOpt = getWord(context, word, 0);
  if (!firstOpt.has_value())
    return false;

  auto first = *firstOpt;

  switch (type)
  {
  case InternalFunctionType::Int__IntArray:
  case InternalFunctionType::Int__Int:
  case InternalFunctionType::Int__Double:
    return isIntegerType(first);
  case InternalFunctionType::IntArray__Int:
  case InternalFunctionType::IntArray__IntArray:
    return first.type == WordType::IntegerArray;
  case InternalFunctionType::Double__Double:
    return (isDoubleType(first) || isIntegerType(first));
  case InternalFunctionType::DoubleArray__Int:
  case InternalFunctionType::DoubleArray__Double:
  case InternalFunctionType::DoubleArray__DoubleArray:
    return first.type == WordType::DoubleArray;
  case InternalFunctionType::Bool__Bool:
    return isBooleanType(first);
  case InternalFunctionType::BoolArray__Int:
  case InternalFunctionType::BoolArray__Bool:
  case InternalFunctionType::BoolArray__BoolArray:
    return first.type == WordType::BooleanArray;
  case InternalFunctionType::Int_Int__Int:
  case InternalFunctionType::Int_Int_Bool:
  case InternalFunctionType::Bool_Bool__Bool:
  case InternalFunctionType::Double_Double__Double:
  case InternalFunctionType::Double_Double_Bool:
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
    break; // two arguments, will be checked below.
  };

  auto secondOpt = getWord(context, word, 1);
  if (!secondOpt.has_value())
    return false;

  auto second = *secondOpt;

  switch (type)
  {
  case InternalFunctionType::Int__IntArray:
  case InternalFunctionType::Int__Int:
  case InternalFunctionType::Int__Double:
  case InternalFunctionType::IntArray__Int:
  case InternalFunctionType::IntArray__IntArray:
  case InternalFunctionType::Double__Double:
  case InternalFunctionType::DoubleArray__Int:
  case InternalFunctionType::DoubleArray__Double:
  case InternalFunctionType::DoubleArray__DoubleArray:
  case InternalFunctionType::Bool__Bool:
  case InternalFunctionType::BoolArray__Int:
  case InternalFunctionType::BoolArray__Bool:
  case InternalFunctionType::BoolArray__BoolArray:
    throw "Fatal error: (checkOverloadCompatibility) single argument function, should have already handled.";
  case InternalFunctionType::Int_Int__Int:
  case InternalFunctionType::Int_Int_Bool:
    return isIntegerType(first) && isIntegerType(second);
  case InternalFunctionType::Bool_Bool__Bool:
    return isBooleanType(first) && isBooleanType(second);
  case InternalFunctionType::Double_Double__Double:
  case InternalFunctionType::Double_Double_Bool: {
    if (isDoubleType(first) && isDoubleType(second))
      return true;
    // Automatic int to double conversions
    return (isIntegerType(first) && isDoubleType(second)) || (isIntegerType(second) && isDoubleType(first));
  }
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
    return hasFunctionCategory(context, first, InternalFunctionCategory::BinaryOptInteger__Integer) &&
           second.type == WordType::IntegerArray;
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
    return hasFunctionCategory(context, first, InternalFunctionCategory::BinaryOptDouble__Double) &&
           second.type == WordType::DoubleArray;
  };

  return false;
}

template <typename R, typename T>
auto foldSingleArgumentNoRankPolyFunction(anka::Context &context, const anka::Word &input,
                                          const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  if (getWordCount(context, input) != 1)
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

  if (getWordCount(context, input) != 2)
    return std::nullopt;

  auto first = getWord(context, input, 0).value();
  auto second = getWord(context, input, 1).value();

  if (isDoubleType(first) && isDoubleType(second))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, double, double>(context, input, func);

  if (isDoubleType(first) && isIntegerType(second))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, double, int, FuncType>(context, input, func);

  if (isDoubleType(second) && isIntegerType(first))
    return foldTwoArgumentWithRankPolyFunction<ReturnType, int, double, FuncType>(context, input, func);

  return std::nullopt;
}

template <typename R, typename T, typename FuncType = R (*)(T)>
auto foldSingleArgumentWithRankPolyFunction(anka::Context &context, const anka::Word &input,
                                            const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  if (getWordCount(context, input) != 1)
    return std::nullopt;

  auto funcptr = (FuncType)func.ptr;

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

auto foldDouble__DoubleVariations(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
    -> std::optional<anka::Word>
{
  if (getWordCount(context, input) != 1)
    return std::nullopt;

  typedef double (*FuncType)(double);
  auto first = getWord(context, input, 0).value();

  if (isDoubleType(first))
    return foldSingleArgumentWithRankPolyFunction<double, double>(context, input, func);

  if (isIntegerType(first))
    return foldSingleArgumentWithRankPolyFunction<double, int, FuncType>(context, input, func);

  return std::nullopt;
}

template <typename R, typename T1, typename T2, typename FuncType = R (*)(T1, T2)>
auto foldTwoArgumentWithRankPolyFunction(anka::Context &context, const anka::Word &input,
                                         const anka::InternalFunction &func) -> std::optional<anka::Word>
{
  if (getWordCount(context, input) != 2)
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
auto foldBinaryOptWithArray(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func,
                            const anka::InternalFunctionCategory category) -> std::optional<anka::Word>
{
  if (getWordCount(context, input) != 2)
    return std::nullopt;

  auto opt1 = getWord(context, input, 0);
  if (!opt1)
    return std::nullopt;

  auto binaryOpOpt = findFunctionWithCategory(context, *opt1, category);
  if (!binaryOpOpt)
    return std::nullopt;

  auto binaryOpt = (anka::BinaryOpt<R, T>)binaryOpOpt.value().ptr;

  auto optVec2 = extractValue<const std::vector<T> &>(context, input, 1);
  if (!optVec2)
    return std::nullopt;

  auto &&vec = *optVec2;

  using AlgorithmFunc = R (*)(BinaryOpt<R, T>, const std::vector<T> &);
  auto algo = (AlgorithmFunc)func.ptr;

  auto ret = algo(binaryOpt, vec);
  return createWord(context, ret);
}

export auto foldFunction(anka::Context &context, const anka::Word &input, const anka::InternalFunction &func)
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
    return foldDouble__DoubleVariations(context, input, func);
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
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
    return foldBinaryOptWithArray<int, int>(context, input, func, InternalFunctionCategory::BinaryOptInteger__Integer);
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
    return foldBinaryOptWithArray<double, double>(context, input, func,
                                                  InternalFunctionCategory::BinaryOptDouble__Double);
  }
  return std::nullopt;
}

} // namespace anka
