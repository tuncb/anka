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

auto inc(int n) -> int
{
  return n + 1;
}

auto dec(int n) -> int
{
  return n - 1;
}

auto neg(int n) -> int
{
  return -n;
}

auto abs(int n) -> int
{
  return std::abs(n);
}

template <typename T> auto length(const std::vector<T> &vec) -> int
{
  return static_cast<int>(vec.size());
}

auto sort(const std::vector<int> &vec) -> std::vector<int>
{
  auto res = vec;
  std::sort(res.begin(), res.end());
  return res;
}

auto add(int v1, int v2) -> int
{
  return v1 + v2;
}

auto sub(int v1, int v2) -> int
{
  return v1 - v2;
}

auto mul(int v1, int v2) -> int
{
  return v1 * v2;
}

auto div(int v1, int v2) -> int
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
  map["inc"] = {{&anka::inc, InternalFunctionType::Int__Int}};
  map["dec"] = {{&anka::dec, InternalFunctionType::Int__Int}};
  map["neg"] = {{&anka::neg, InternalFunctionType::Int__Int}};
  map["abs"] = {{&anka::abs, InternalFunctionType::Int__Int}};
  map["length"] = {{&anka::length<int>, InternalFunctionType::IntArray__Int},
                   {&anka::length<bool>, InternalFunctionType::BoolArray__Int}};
  map["sort"] = {{&anka::sort, InternalFunctionType::IntArray__IntArray}};
  map["add"] = {{&anka::add, InternalFunctionType::Int_Int__Int}};
  map["sub"] = {{&anka::sub, InternalFunctionType::Int_Int__Int}};
  map["mul"] = {{&anka::mul, InternalFunctionType::Int_Int__Int}};
  map["div"] = {{&anka::div, InternalFunctionType::Int_Int__Int}};
  map["and"] = {{&anka::andFun, InternalFunctionType::Bool_Bool__Bool}};
  map["or"] = {{&anka::orFun, InternalFunctionType::Bool_Bool__Bool}};
  map["equals"] = {{&anka::equals<int>, InternalFunctionType::Int_Int_Bool},
                   {&anka::equals<bool>, InternalFunctionType::Bool_Bool__Bool}};
  map["not_equals"] = {{&anka::notEquals<int>, InternalFunctionType::Int_Int_Bool},
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
  case InternalFunctionType::Bool__Bool:
    return "bool -> bool";
  case InternalFunctionType::IntArray__Int:
    return "(int) -> int";
  case InternalFunctionType::IntArray__IntArray:
    return "(int) -> (int)";
  case InternalFunctionType::Int_Int__Int:
    return "[int int] -> int";
  case InternalFunctionType::Int_Int_Bool:
    return "[int int] -> bool";
  case InternalFunctionType::Bool_Bool__Bool:
    return "[bool bool] -> bool";
  case InternalFunctionType::BoolArray__Int:
    return "(bool) -> bool";
  }

  throw std::runtime_error("Fatal Error: Unexpected internal function type in toString function");
}