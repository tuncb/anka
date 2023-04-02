module;
#include <algorithm>
#include <numbers>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
export module anka:internal_functions;

namespace anka
{

export enum class InternalFunctionType
{
  Int__IntArray,
  Int__Int,
  Int_Int__Int,
  Int_Int_Bool,
  Int__Double,
  IntArray__Int,
  IntArray__IntArray,
  Bool__Bool,
  Bool_Bool__Bool,
  BoolArray__Int,
  BoolArray__BoolArray,
  BoolArray__Bool,
  Double__Double,
  Double_Double__Double,
  Double_Double_Bool,
  DoubleArray__Int,
  DoubleArray__Double,
  DoubleArray__DoubleArray,
  IntBinaryOpt_IntArray__Int,
  DoubleBinaryOpt_DoubleArray__Double,
  IntBinaryOpt_IntArray__IntArray,
  DoubleBinaryOpt_DoubleArray__DoubleArray,
};

export auto toString(InternalFunctionType type) -> std::string
{
  switch (type)
  {
  case InternalFunctionType::Int__IntArray:
    return "int -> (int)";
  case InternalFunctionType::Int__Int:
    return "int -> int";
  case InternalFunctionType::Int__Double:
    return "int -> double";
  case InternalFunctionType::Int_Int__Int:
    return "[int int] -> int";
  case InternalFunctionType::Int_Int_Bool:
    return "[int int] -> bool";
  case InternalFunctionType::IntArray__Int:
    return "(int) -> int";
  case InternalFunctionType::IntArray__IntArray:
    return "(int) -> (int)";
  case InternalFunctionType::Bool__Bool:
    return "bool -> bool";
  case InternalFunctionType::Bool_Bool__Bool:
    return "[bool bool] -> bool";
  case InternalFunctionType::BoolArray__Int:
    return "(bool) -> int";
  case InternalFunctionType::BoolArray__Bool:
    return "(bool) -> bool";
  case InternalFunctionType::BoolArray__BoolArray:
    return "(bool) -> (bool)";
  case InternalFunctionType::Double__Double:
    return "double -> double";
  case InternalFunctionType::Double_Double__Double:
    return "[double double] -> double";
  case InternalFunctionType::Double_Double_Bool:
    return "[double double] -> bool";
  case InternalFunctionType::DoubleArray__Int:
    return "(double) -> int";
  case InternalFunctionType::DoubleArray__DoubleArray:
    return "(double) -> (double)";
  case InternalFunctionType::DoubleArray__Double:
    return "(double) -> double";
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
    return "{int int -> int} (int) -> int";
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
    return "{double double -> double} (double) -> double";
  case InternalFunctionType::IntBinaryOpt_IntArray__IntArray:
    return "{int int -> int} (int) -> (int)";
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__DoubleArray:
    return "{double double -> double} (double) -> (double)";
  }

  throw std::runtime_error("Fatal Error: Unexpected internal function type in toString function");
}

export auto nrArguments(InternalFunctionType type) -> int
{
  switch (type)
  {
  case InternalFunctionType::Int__IntArray:
  case InternalFunctionType::Int__Int:
  case InternalFunctionType::Int__Double:
  case InternalFunctionType::IntArray__Int:
  case InternalFunctionType::IntArray__IntArray:
  case InternalFunctionType::Bool__Bool:
  case InternalFunctionType::Double__Double:
  case InternalFunctionType::BoolArray__Int:
  case InternalFunctionType::BoolArray__Bool:
  case InternalFunctionType::BoolArray__BoolArray:
  case InternalFunctionType::DoubleArray__Int:
  case InternalFunctionType::DoubleArray__DoubleArray:
  case InternalFunctionType::DoubleArray__Double:
    return 1;
  case InternalFunctionType::Int_Int__Int:
  case InternalFunctionType::Int_Int_Bool:
  case InternalFunctionType::Bool_Bool__Bool:
  case InternalFunctionType::Double_Double__Double:
  case InternalFunctionType::Double_Double_Bool:
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
  case InternalFunctionType::IntBinaryOpt_IntArray__IntArray:
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__DoubleArray:
    return 2;
  }

  throw std::runtime_error("Fatal Error: Unexpected internal function type in nrArguments function");
}

export struct InternalFunction
{
  void *ptr = nullptr;
  InternalFunctionType type;
};

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

export template <typename R, typename T> using BinaryOpt = R(*)(T, T);

template <typename T, typename R> auto foldl(BinaryOpt<T, R> func, const std::vector<T> &vec) -> R
{
  if (vec.empty())
    return (R)0;
  return std::accumulate(vec.begin() + 1, vec.end(), vec.front(), func);
}

template <typename T, typename R> auto scanl(BinaryOpt<T, R> func, const std::vector<T> &vec) -> std::vector<R>
{
  if (vec.empty())
    return vec;
  std::vector<R> res;
  res.resize(vec.size());
  std::partial_sum(vec.begin(), vec.end(), res.begin(), func);
  return res;
}

export auto getInternalFunctions() -> const std::unordered_map<std::string, std::vector<anka::InternalFunction>> &
{
  static std::optional<std::unordered_map<std::string, std::vector<anka::InternalFunction>>> functionMapOpt;
  if (functionMapOpt.has_value())
    return functionMapOpt.value();

  typedef double (*DoubleToDoubleFunc)(double);

  std::unordered_map<std::string, std::vector<anka::InternalFunction>> map;
  map["ioata"] = {{&anka::ioata, InternalFunctionType::Int__IntArray}};
  map["inc"] = {{&anka::inc<int>, InternalFunctionType::Int__Int},
                {&anka::inc<double>, InternalFunctionType::Double__Double}};
  map["dec"] = {{&anka::dec<int>, InternalFunctionType::Int__Int},
                {&anka::dec<double>, InternalFunctionType::Double__Double}};
  map["neg"] = {{&anka::neg<int>, InternalFunctionType::Int__Int},
                {&anka::neg<double>, InternalFunctionType::Double__Double}};
  map["abs"] = {{&anka::abs<int>, InternalFunctionType::Int__Int},
                {&anka::abs<double>, InternalFunctionType::Double__Double}};
  map["sqrt"] = {{(DoubleToDoubleFunc)&std::sqrt, InternalFunctionType::Double__Double}};
  map["exp"] = {{(DoubleToDoubleFunc)&std::exp, InternalFunctionType::Double__Double}};
  map["log"] = {{(DoubleToDoubleFunc)&std::log, InternalFunctionType::Double__Double}};
  map["log10"] = {{(DoubleToDoubleFunc)&std::log10, InternalFunctionType::Double__Double}};
  map["sin"] = {{(DoubleToDoubleFunc)&std::sin, InternalFunctionType::Double__Double}};
  map["cos"] = {{(DoubleToDoubleFunc)&std::cos, InternalFunctionType::Double__Double}};
  map["tan"] = {{(DoubleToDoubleFunc)&std::tan, InternalFunctionType::Double__Double}};
  map["floor"] = {{(DoubleToDoubleFunc)&std::floor, InternalFunctionType::Double__Double}};
  map["ceil"] = {{(DoubleToDoubleFunc)&std::ceil, InternalFunctionType::Double__Double}};
  map["trunc"] = {{(DoubleToDoubleFunc)&std::trunc, InternalFunctionType::Double__Double}};
  map["length"] = {{&anka::length<int>, InternalFunctionType::IntArray__Int},
                   {&anka::length<double>, InternalFunctionType::DoubleArray__Int},
                   {&anka::length<bool>, InternalFunctionType::BoolArray__Int}};
  map["sort"] = {{&anka::sort<int>, InternalFunctionType::IntArray__IntArray},
                 {&anka::sort<bool>, InternalFunctionType::BoolArray__BoolArray},
                 {&anka::sort<double>, InternalFunctionType::DoubleArray__DoubleArray}};
  map["add"] = {{&anka::add<int>, InternalFunctionType::Int_Int__Int},
                {&anka::add<double>, InternalFunctionType::Double_Double__Double}};
  map["sub"] = {{&anka::sub<int>, InternalFunctionType::Int_Int__Int},
                {&anka::sub<double>, InternalFunctionType::Double_Double__Double}};
  map["mul"] = {{&anka::mul<int>, InternalFunctionType::Int_Int__Int},
                {&anka::mul<double>, InternalFunctionType::Double_Double__Double}};
  map["div"] = {{&anka::div<int>, InternalFunctionType::Int_Int__Int},
                {&anka::div<double>, InternalFunctionType::Double_Double__Double}};
  map["and"] = {{&anka::andFun, InternalFunctionType::Bool_Bool__Bool}};
  map["or"] = {{&anka::orFun, InternalFunctionType::Bool_Bool__Bool}};
  map["equals"] = {{&anka::equals<int>, InternalFunctionType::Int_Int_Bool},
                   {&anka::equals<double>, InternalFunctionType::Double_Double_Bool},
                   {&anka::equals<bool>, InternalFunctionType::Bool_Bool__Bool}};
  map["not_equals"] = {{&anka::notEquals<int>, InternalFunctionType::Int_Int_Bool},
                       {&anka::notEquals<double>, InternalFunctionType::Double_Double_Bool},
                       {&anka::notEquals<bool>, InternalFunctionType::Bool_Bool__Bool}};
  map["not"] = {{&anka::notFun, InternalFunctionType::Bool__Bool}};
  map["all_of"] = {{&anka::all_of, InternalFunctionType::BoolArray__Bool}};
  map["any_of"] = {{&anka::any_of, InternalFunctionType::BoolArray__Bool}};
  map["none_of"] = {{&anka::none_of, InternalFunctionType::BoolArray__Bool}};
  map["sum"] = {{&anka::sum<int>, InternalFunctionType::IntArray__Int},
                {&anka::sum<double>, InternalFunctionType::DoubleArray__Double}};
  map["to_double"] = {{&anka::to_double<int>, InternalFunctionType::Int__Double},
                      {&anka::to_double<double>, InternalFunctionType::Double__Double}};
  map["foldl"] = {
      {&anka::foldl<int, int>, InternalFunctionType::IntBinaryOpt_IntArray__Int},
      {&anka::foldl<double, double>, InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double},
  };
  map["scanl"] = {
      {&anka::scanl<int, int>, InternalFunctionType::IntBinaryOpt_IntArray__IntArray},
      {&anka::scanl<double, double>, InternalFunctionType::DoubleBinaryOpt_DoubleArray__DoubleArray},
  };

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

export enum class InternalFunctionCategory
{
  BinaryOptInteger__Integer,
  BinaryOptDouble__Double,
  Uncategorized
};

export auto getCategory(anka::InternalFunctionType type) -> InternalFunctionCategory
{
  switch (type)
  {
  case InternalFunctionType::Int_Int__Int:
    return InternalFunctionCategory::BinaryOptInteger__Integer;
  case InternalFunctionType::Double_Double__Double:
    return InternalFunctionCategory::BinaryOptDouble__Double;
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
  case InternalFunctionType::Int_Int_Bool:
  case InternalFunctionType::Bool_Bool__Bool:
  case InternalFunctionType::Double_Double_Bool:
  case InternalFunctionType::IntBinaryOpt_IntArray__Int:
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__Double:
  case InternalFunctionType::IntBinaryOpt_IntArray__IntArray:
  case InternalFunctionType::DoubleBinaryOpt_DoubleArray__DoubleArray:
    return InternalFunctionCategory::Uncategorized;
  };

  throw std::runtime_error("Fatal Error: Unexpected internal function type in getCategory function");
}

} // namespace anka
