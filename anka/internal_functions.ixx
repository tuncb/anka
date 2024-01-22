module;
#include <algorithm>
#include <functional>
#include <numbers>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tl/optional.hpp>

export module anka:internal_functions;

import :errors;
import :type_system;
import :interpreter_state;

namespace anka
{

template <typename R, typename T> using BinaryOpt = R (*)(T, T);
template <typename T> using FilterFunc = bool (*)(T);

auto ioata(int n) -> std::vector<int>
{
  std::vector<int> res;
  if (n < 1)
    return res;

  res.reserve(n);
  for (auto i = 0; i < n; ++i)
  {
    res.push_back(i + 1);
  }
  return res;
}

template <typename T> auto inc(T n) -> T
{
  return n + (T)1;
}

template <typename T> auto dec(T n) -> T
{
  return n - (T)1;
}

auto odd(int n) -> bool
{
  return n % 2 == 1;
}

auto even(int n) -> bool
{
  return n % 2 == 0;
}

template <typename T> auto is_positive(T n) -> bool
{
  return n > 0;
}

template <typename T> auto is_negative(T n) -> bool
{
  return n < 0;
}

template <typename T> auto neg(T n) -> T
{
  return -n;
}
template <typename T> auto abs(T n) -> T
{
  return std::abs(n);
}

template <typename T> auto length(const std::vector<T> &vec) -> int
{
  return static_cast<int>(vec.size());
}

template <typename T> auto sort(const std::vector<T> &vec) -> std::vector<T>
{
  auto res = vec;
  std::sort(res.begin(), res.end());
  return res;
}

template <typename T> auto add(T v1, T v2) -> T
{
  return v1 + v2;
}

template <typename T> auto sub(T v1, T v2) -> T
{
  return v1 - v2;
}

template <typename T> auto mul(T v1, T v2) -> T
{
  return v1 * v2;
}
template <typename T> auto div(T v1, T v2) -> T
{
  return v1 / v2;
}

auto andFun(bool b1, bool b2) -> bool
{
  return b1 and b2;
}

auto orFun(bool b1, bool b2) -> bool
{
  return b1 or b2;
}

auto notFun(bool b) -> bool
{
  return !b;
}

template <typename T> auto equals(T v1, T v2) -> bool
{
  return v1 == v2;
}

template <typename T> auto notEquals(T v1, T v2) -> bool
{
  return v1 != v2;
}

template <typename T> auto sum(const std::vector<T> &vec) -> T
{
  return std::accumulate(vec.begin(), vec.end(), (T)0);
}

auto all_of(const std::vector<bool> &vec) -> bool
{
  return std::all_of(vec.begin(), vec.end(), std::identity());
}

auto any_of(const std::vector<bool> &vec) -> bool
{
  return std::any_of(vec.begin(), vec.end(), std::identity());
}

auto none_of(const std::vector<bool> &vec) -> bool
{
  return std::none_of(vec.begin(), vec.end(), std::identity());
}

template <typename T> auto to_double(T val) -> double
{
  return val;
}

template <typename T, typename R> auto foldl(anka::BinaryOpt<T, R> func, const std::vector<T> &vec) -> R
{
  if (vec.empty())
    return (R)0;
  return std::accumulate(vec.begin() + 1, vec.end(), vec.front(), func);
}

template <typename T, typename R> auto scanl(anka::BinaryOpt<T, R> func, const std::vector<T> &vec) -> std::vector<R>
{
  if (vec.empty())
    return vec;
  std::vector<R> res;
  res.resize(vec.size());
  std::partial_sum(vec.begin(), vec.end(), res.begin(), func);
  return res;
}

template <typename T> auto filter(anka::FilterFunc<T> func, const std::vector<T> &vec) -> std::vector<T>
{
  std::vector<T> ret;
  if (vec.empty())
    return ret;

  std::copy_if(vec.begin(), vec.end(), std::back_inserter(ret), func);

  return ret;
}

export using InternalFunctionExecuter = std::function<std::optional<anka::Word>(
    anka::Context &, const std::vector<anka::Word> &, const std::vector<bool> &expandArray)>;

export struct InternalFunctionDefinition
{
  std::string name;
  TypeList argumentTypes;
  TypeVariant returnType;
  void *funcPtr = nullptr;
};

export struct InternalFunctionDefinitionHash
{
  inline size_t operator()(const InternalFunctionDefinition &k) const
  {
    return std::hash<std::string>()(k.name) ^ anka::hash(k.argumentTypes);
  }
};

export struct InternalFunctionDefinitionEqual
{
  inline bool operator()(const InternalFunctionDefinition &k1, const InternalFunctionDefinition &k2) const
  {
    return (k1.name == k2.name) && (k1.argumentTypes == k2.argumentTypes);
  }
};

export using InternalFunctionMaptype =
    std::unordered_map<InternalFunctionDefinition, InternalFunctionExecuter, InternalFunctionDefinitionHash,
                       InternalFunctionDefinitionEqual>;

auto getInternalFunctions() -> const InternalFunctionMaptype &;

export auto getAllInternalFunctionDefinitions() -> std::vector<InternalFunctionDefinition>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  auto kv = ranges::views::keys(internalFunctions);
  std::vector<anka::InternalFunctionDefinition> definitions{kv.begin(), kv.end()};
  return definitions;
}

template <typename T>
auto getInternalFunctionDefinition(const std::string &name) -> std::optional<InternalFunctionDefinition>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  auto type = anka::getType<T>();
  auto funcType = std::get<anka::FunctionType>(type);
  const auto arguments = std::vector<TypeVariant>(funcType.arguments.begin(), funcType.arguments.end());
  const auto definition = anka::InternalFunctionDefinition{name, arguments, funcType.returnType, nullptr};
  auto iter = internalFunctions.find(definition);

  if (iter == internalFunctions.end())
  {
    return std::nullopt;
  }

  return iter->first;
}

export auto getInternalFunction(const std::string &name, const std::vector<anka::TypeVariant> &arguments)
    -> std::optional<std::pair<InternalFunctionDefinition, InternalFunctionExecuter>>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  // anka::WordType::Name is a dummy => fix this!!!, do we really need the result type in the definition?
  const auto definition = anka::InternalFunctionDefinition{name, arguments, anka::TypeFamily::Void, nullptr};

  auto iter = internalFunctions.find(definition);
  if (iter == internalFunctions.end())
    return std::nullopt;

  return std::make_pair(iter->first, iter->second);
}

export auto toString(InternalFunctionDefinition definition) -> std::string
{
  auto argText = anka::toString(definition.argumentTypes);

  return fmt::format("[{}] -> {}", fmt::join(argText, ", "), anka::toString({definition.returnType}));
}

template <typename T>
auto getValueWithConversion(anka::Context &context, anka::Word word) -> ValueReturnType<T>::ReturnType
{
  if constexpr (std::is_same_v<T, double>)
  {
    if (word.type == WordType::IntegerNumber)
    {
      auto iVal = anka::getValue<int>(context, word.index);
      return static_cast<T>(iVal);
    }
    return anka::getValue<T>(context, word.index);
  }
  else
  {
    return anka::getValue<T>(context, word.index);
  }
}

template <typename T>
auto getValue(anka::Context &context, const std::vector<anka::Word> &words, const std::vector<bool> &expandArray,
              size_t arrIndex, size_t index) -> ValueReturnType<T>::ReturnType
{
  auto word = words[index];

  // Internal function as argument
  if constexpr (std::is_function_v<typename std::remove_pointer<T>::type>)
  {
    if (word.type != WordType::Name)
    {
      throw anka::ExecutionError{word, std::nullopt, "Expected a function"};
    }

    auto funcName = anka::getValueWithConversion<std::string>(context, word);
    auto def = getInternalFunctionDefinition<T>(funcName);

    if (!def)
    {
      throw anka::ExecutionError{word, std::nullopt, "Expected a function"};
    }

    return reinterpret_cast<T>(def.value().funcPtr);
  }
  else if constexpr (anka::isExpandable<T>())
  {
    auto shouldExpandArray = expandArray[index];

    if (shouldExpandArray)
    {
      auto &&vec = anka::getValue<std::vector<T>>(context, word.index);
      if (arrIndex >= vec.size())
      {
        throw anka::ExecutionError{word, std::nullopt, "Array size mismatch"};
      }
      return vec[arrIndex];
    }

    return anka::getValueWithConversion<T>(context, word);
  }
  else
  {
    return anka::getValueWithConversion<T>(context, word);
  }
}

template <typename T>
auto getArgumentSize(anka::Context &context, const std::vector<anka::Word> &words, const std::vector<bool> &expandArray,
                     size_t index) -> size_t
{
  if (!expandArray[index])
    return 1;

  return anka::getItemSize<std::vector<T>>(context, words[index].index);
}

template <typename... ArgTypes>
auto createArguments(anka::Context &context, const std::vector<anka::Word> &words, const std::vector<bool> &expandArray,
                     size_t arrIndex) -> std::tuple<ArgTypes...>
{
  // see https://stackoverflow.com/questions/65261797/varadic-template-to-tuple-is-reversed
  // using an initilizer list fixes the order
  size_t i = 0;
  auto args = std::tuple<ArgTypes...>{getValue<ArgTypes>(context, words, expandArray, arrIndex, i++)...};
  return args;
}

template <typename ReturnType, typename... ArgTypes>
auto createFunctionExecutor(void *funPtr) -> InternalFunctionExecuter
{
  typedef ReturnType (*FunType)(ArgTypes...);
  FunType func = static_cast<FunType>(funPtr);

  auto executor = [func](anka::Context &context, const std::vector<anka::Word> &words,
                         const std::vector<bool> &expandArray) -> std::optional<anka::Word> {
    if constexpr (!anka::isExpandable<ReturnType>())
    {
      auto args = createArguments<ArgTypes...>(context, words, expandArray, 0);
      if constexpr (std::is_same<ReturnType, void>::value)
      {
        std::apply(func, args);
        return std::nullopt;
      }
      else
      {
        auto ret = std::apply(func, args);
        return anka::createWord(context, std::move(ret));
      }
    }
    else
    {
      size_t i = 0;
      auto sizes = std::vector<size_t>{getArgumentSize<ArgTypes>(context, words, expandArray, i++)...};
      auto max_size = std::ranges::max(sizes);

      if (max_size == 1)
      {
        auto args = createArguments<ArgTypes...>(context, words, expandArray, 0);
        if constexpr (std::is_same<ReturnType, void>::value)
        {
          std::apply(func, args);
          return std::nullopt;
        }
        else
        {
          auto ret = std::apply(func, args);
          return anka::createWord(context, std::move(ret));
        }
      }

      if constexpr (std::is_same<ReturnType, void>::value)
      {
        for (auto arrIndex = 0; arrIndex < max_size; ++arrIndex)
        {
          auto args = createArguments<ArgTypes...>(context, words, expandArray, arrIndex);
          std::apply(func, args);
        }
        return std::nullopt;
      }
      else
      {
        std::vector<ReturnType> vec(max_size);
        for (auto arrIndex = 0; arrIndex < max_size; ++arrIndex)
        {
          auto args = createArguments<ArgTypes...>(context, words, expandArray, arrIndex);
          vec[arrIndex] = std::apply(func, args);
        }
        return anka::createWord(context, std::move(vec));
      }
    }
  };

  return executor;
}

template <typename T> anka::WordType getValueWithInternalFunctions()
{
  using Decayed = std::decay<T>::type;

  if constexpr (std::is_same_v<Decayed, anka::BinaryOpt<int, int>>)
    return anka::WordType::Name;
  else if constexpr (std::is_same_v<Decayed, anka::BinaryOpt<double, double>>)
    return anka::WordType::Name;
  else
    return anka::getWordType<T>;
}

template <typename ReturnType, typename... ArgTypes>
auto addInternalFunction(InternalFunctionMaptype &map, std::string &&name, void *ptr)
{
  InternalFunctionDefinition def;
  def.name = name;
  def.returnType = anka::getType<ReturnType>();
  def.argumentTypes = std::vector<anka::TypeVariant>{anka::getType<ArgTypes>()...};
  def.funcPtr = ptr;

  map[def] = createFunctionExecutor<ReturnType, ArgTypes...>(ptr);
}

export auto getInternalFunctions() -> const InternalFunctionMaptype &
{
  static std::optional<InternalFunctionMaptype> functionMapOpt;

  if (functionMapOpt.has_value())
    return functionMapOpt.value();

  InternalFunctionMaptype map;
  addInternalFunction<std::vector<int>, int>(map, "ioata", &anka::ioata);

  addInternalFunction<bool, int>(map, "odd", &anka::odd);
  addInternalFunction<bool, int>(map, "even", &anka::even);
  addInternalFunction<bool, int>(map, "is_positive", &anka::is_positive<int>);
  addInternalFunction<bool, double>(map, "is_positive", &anka::is_positive<double>);
  addInternalFunction<bool, int>(map, "is_negative", &anka::is_negative<int>);
  addInternalFunction<bool, double>(map, "is_negative", &anka::is_negative<double>);

  addInternalFunction<int, int>(map, "inc", &anka::inc<int>);
  addInternalFunction<double, double>(map, "inc", &anka::inc<double>);
  addInternalFunction<int, int>(map, "dec", &anka::dec<int>);
  addInternalFunction<double, double>(map, "dec", &anka::dec<double>);
  addInternalFunction<int, int>(map, "neg", &anka::neg<int>);
  addInternalFunction<double, double>(map, "neg", &anka::neg<double>);
  addInternalFunction<int, int>(map, "abs", &anka::abs<int>);
  addInternalFunction<double, double>(map, "abs", &anka::abs<double>);

  typedef double (*DoubleToDoubleFunc)(double);
  addInternalFunction<double, double>(map, "sqrt", static_cast<DoubleToDoubleFunc>(&std::sqrt));
  addInternalFunction<double, double>(map, "exp", static_cast<DoubleToDoubleFunc>(&std::exp));
  addInternalFunction<double, double>(map, "log", static_cast<DoubleToDoubleFunc>(&std::log));
  addInternalFunction<double, double>(map, "log10", static_cast<DoubleToDoubleFunc>(&std::log10));
  addInternalFunction<double, double>(map, "sin", static_cast<DoubleToDoubleFunc>(&std::sin));
  addInternalFunction<double, double>(map, "cos", static_cast<DoubleToDoubleFunc>(&std::cos));
  addInternalFunction<double, double>(map, "tan", static_cast<DoubleToDoubleFunc>(&std::tan));
  addInternalFunction<double, double>(map, "floor", static_cast<DoubleToDoubleFunc>(&std::floor));
  addInternalFunction<double, double>(map, "ceil", static_cast<DoubleToDoubleFunc>(&std::ceil));
  addInternalFunction<double, double>(map, "trunc", static_cast<DoubleToDoubleFunc>(&std::trunc));

  addInternalFunction<int, std::vector<int>>(map, "length", &anka::length<int>);
  addInternalFunction<int, std::vector<double>>(map, "length", &anka::length<double>);
  addInternalFunction<int, std::vector<bool>>(map, "length", &anka::length<bool>);

  addInternalFunction<std::vector<int>, std::vector<int>>(map, "sort", &anka::sort<int>);
  addInternalFunction<std::vector<double>, std::vector<double>>(map, "sort", &anka::sort<double>);
  addInternalFunction<std::vector<bool>, std::vector<bool>>(map, "sort", &anka::sort<bool>);

  addInternalFunction<int, int, int>(map, "add", &anka::add<int>);
  addInternalFunction<double, double, double>(map, "add", &anka::add<double>);
  addInternalFunction<int, int, int>(map, "sub", &anka::sub<int>);
  addInternalFunction<double, double, double>(map, "sub", &anka::sub<double>);
  addInternalFunction<int, int, int>(map, "mul", &anka::mul<int>);
  addInternalFunction<double, double, double>(map, "mul", &anka::mul<double>);
  addInternalFunction<int, int, int>(map, "div", &anka::div<int>);
  addInternalFunction<double, double, double>(map, "div", &anka::div<double>);

  addInternalFunction<bool, bool, bool>(map, "and", &anka::andFun);
  addInternalFunction<bool, bool, bool>(map, "or", &anka::orFun);

  addInternalFunction<bool, bool, bool>(map, "equals", &anka::equals<bool>);
  addInternalFunction<bool, int, int>(map, "equals", &anka::equals<int>);
  addInternalFunction<bool, double, double>(map, "equals", &anka::equals<double>);
  addInternalFunction<bool, bool, bool>(map, "not_equals", &anka::notEquals<bool>);
  addInternalFunction<bool, int, int>(map, "not_equals", &anka::notEquals<int>);
  addInternalFunction<bool, double, double>(map, "not_equals", &anka::notEquals<double>);

  addInternalFunction<bool, bool>(map, "not", &anka::notFun);
  addInternalFunction<bool, std::vector<bool>>(map, "all_of", &anka::all_of);
  addInternalFunction<bool, std::vector<bool>>(map, "any_of", &anka::any_of);
  addInternalFunction<bool, std::vector<bool>>(map, "none_of", &anka::none_of);

  addInternalFunction<int, std::vector<int>>(map, "sum", &anka::sum<int>);
  addInternalFunction<double, std::vector<double>>(map, "sum", &anka::sum<double>);

  addInternalFunction<int, double>(map, "to_double", &anka::to_double<int>);
  addInternalFunction<double, double>(map, "to_double", &anka::to_double<double>);

  addInternalFunction<bool, anka::BinaryOpt<bool, bool>, std::vector<bool>>(map, "foldl", &anka::foldl<bool, bool>);
  addInternalFunction<int, anka::BinaryOpt<int, int>, std::vector<int>>(map, "foldl", &anka::foldl<int, int>);
  addInternalFunction<double, anka::BinaryOpt<double, double>, std::vector<double>>(map, "foldl",
                                                                                    &anka::foldl<double, double>);

  addInternalFunction<std::vector<bool>, anka::BinaryOpt<bool, bool>, std::vector<bool>>(map, "scanl",
                                                                                         &anka::scanl<bool, bool>);
  addInternalFunction<std::vector<int>, anka::BinaryOpt<int, int>, std::vector<int>>(map, "scanl",
                                                                                     &anka::scanl<int, int>);
  addInternalFunction<std::vector<double>, anka::BinaryOpt<double, double>, std::vector<double>>(
      map, "scanl", &anka::scanl<double, double>);

  addInternalFunction<std::vector<bool>, anka::FilterFunc<bool>, std::vector<bool>>(map, "filter", &anka::filter<bool>);
  addInternalFunction<std::vector<int>, anka::FilterFunc<int>, std::vector<int>>(map, "filter", &anka::filter<int>);
  addInternalFunction<std::vector<double>, anka::FilterFunc<double>, std::vector<double>>(map, "filter",
                                                                                          &anka::filter<double>);

  functionMapOpt = std::move(map);
  return functionMapOpt.value();
}

// Internal constants

auto getInternalDoubleConstants() -> const std::unordered_map<std::string, double> &
{
  static std::optional<std::unordered_map<std::string, double>> mapOpt;
  if (mapOpt.has_value())
    return mapOpt.value();

  std::unordered_map<std::string, double> map;
  map["pi"] = std::numbers::pi;
  map["e"] = std::numbers::e;

  mapOpt = std::move(map);
  return mapOpt.value();
}

export template <typename T> auto getInternalConstants() -> const std::unordered_map<std::string, T> &
{
  if constexpr (std::is_same_v<T, double>)
  {
    return getInternalDoubleConstants();
  }
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getInternalConstants().");
    }
  ();
}

export auto getInternalFunctionDefinitionsWithName(const std::string &name) -> std::vector<InternalFunctionDefinition>
{
  auto kv = ranges::views::keys(getInternalFunctions());
  std::vector<anka::InternalFunctionDefinition> definitions{kv.begin(), kv.end()};

  return definitions | ranges::views::filter([&name](const auto &def) { return def.name == name; }) |
         ranges::to<std::vector<InternalFunctionDefinition>>;
}

export auto toType(WordType wtype) -> TypeVariant
{
  switch (wtype)
  {
  case WordType::IntegerNumber:
    return TypeFamily::Int;
  case WordType::IntegerArray:
    return TypeFamily::IntArray;
  case WordType::DoubleNumber:
    return TypeFamily::Double;
  case WordType::DoubleArray:
    return TypeFamily::DoubleArray;
  case WordType::Boolean:
    return TypeFamily::Bool;
  case WordType::BooleanArray:
    return TypeFamily::BoolArray;
  case WordType::Name:
    return TypeFamily::Void; // return function variant here
  default:
    throw anka::ExecutionError{std::nullopt, std::nullopt, "Could not understant WordType"};
  }
}

template <typename T> auto injectInternalConstants(anka::Context &context) -> void
{
  auto &&map = anka::getInternalConstants<T>();
  for (auto &&pair : map)
  {
    context.userDefinedNames[pair.first] = createWord(context, pair.second);
  }
}

export auto injectInternalConstants(Context &context) -> void
{
  injectInternalConstants<double>(context);
}


} // namespace anka
