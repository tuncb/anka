#include "internal_functions.h"

#include <algorithm>
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
  }

  throw std::runtime_error("Fatal Error: Unexpected internal function type in toString function");
}