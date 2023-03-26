#include "internal_functions.h"

#include <algorithm>
#include <numeric>
#include <optional>

namespace anka
{

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

auto sqrt(double val) -> double
{
  return std::sqrt(val);
}

auto exp(double val) -> double
{
  return std::exp(val);
}

auto log(double val) -> double
{
  return std::log(val);
}

auto log10(double val) -> double
{
  return std::log10(val);
}

auto sin(double val) -> double
{
  return std::sin(val);
}

auto cos(double val) -> double
{
  return std::cos(val);
}

auto tan(double val) -> double
{
  return std::tan(val);
}

} // namespace anka

auto anka::getInternalFunctions() -> const std::unordered_map<std::string, std::vector<anka::InternalFunction>> &
{
  static std::optional<std::unordered_map<std::string, std::vector<anka::InternalFunction>>> functionMapOpt;
  if (functionMapOpt.has_value())
    return functionMapOpt.value();

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
  map["sqrt"] = {{&anka::sqrt, InternalFunctionType::Double__Double}};
  map["exp"] = {{&anka::exp, InternalFunctionType::Double__Double}};
  map["log"] = {{&anka::log, InternalFunctionType::Double__Double}};
  map["log10"] = {{&anka::log10, InternalFunctionType::Double__Double}};
  map["sin"] = {{&anka::sin, InternalFunctionType::Double__Double}};
  map["cos"] = {{&anka::cos, InternalFunctionType::Double__Double}};
  map["tan"] = {{&anka::tan, InternalFunctionType::Double__Double}};
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

  functionMapOpt = std::move(map);
  return functionMapOpt.value();
}

auto anka::toString(InternalFunctionType type) -> std::string
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
  }

  throw std::runtime_error("Fatal Error: Unexpected internal function type in toString function");
}