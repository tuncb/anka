#include "internal_functions.h"

#include <optional>

auto ioata(int n) -> std::vector<int>
{
  std::vector<int> res;
  if (n < 1)
    return res;

  res.reserve(n);
  for(auto i = 0; i < n; ++i)
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

auto absValue(int n) -> int
{
  return std::abs(n);
}

auto length(const std::vector<int>& vec) -> int
{
  return vec.size();
}

auto anka::getInternalFunctions() -> const std::unordered_map<std::string, anka::InternalFunction> &
{
  static std::optional<std::unordered_map<std::string, anka::InternalFunction>> functionMapOpt;
  if (functionMapOpt.has_value())
    return functionMapOpt.value();

  std::unordered_map<std::string, anka::InternalFunction> map;
  map["ioata"] = {&ioata, InternalFunctionType::IntToIntArray};
  map["inc"] = {&inc, InternalFunctionType::IntToInt};
  map["dec"] = {&dec, InternalFunctionType::IntToInt};
  map["neg"] = {&neg, InternalFunctionType::IntToInt};
  map["abs"] = {&absValue, InternalFunctionType::IntToInt};
  map["length"] = {&length, InternalFunctionType::IntArrayToInt};

  functionMapOpt = std::move(map);
  return functionMapOpt.value();
}