#include "internal_functions.h"

#include <optional>
#include <algorithm>

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

auto length(const std::vector<int> &vec) -> int
{
  return static_cast<int>(vec.size());
}

auto sort(const std::vector<int> &vec) -> std::vector<int>
{
  auto res = vec;
  std::sort(res.begin(), res.end());
  return res;
}

} // namespace anka

auto anka::getInternalFunctions() -> const std::unordered_map<std::string, anka::InternalFunction> &
{
  static std::optional<std::unordered_map<std::string, anka::InternalFunction>> functionMapOpt;
  if (functionMapOpt.has_value())
    return functionMapOpt.value();

  std::unordered_map<std::string, anka::InternalFunction> map;
  map["ioata"] = {&anka::ioata, InternalFunctionType::IntToIntArray};
  map["inc"] = {&anka::inc, InternalFunctionType::IntToInt};
  map["dec"] = {&anka::dec, InternalFunctionType::IntToInt};
  map["neg"] = {&anka::neg, InternalFunctionType::IntToInt};
  map["abs"] = {&anka::abs, InternalFunctionType::IntToInt};
  map["length"] = {&anka::length, InternalFunctionType::IntArrayToInt};
  map["sort"] = {&anka::sort, InternalFunctionType::IntArrayToIntArray};

  functionMapOpt = std::move(map);
  return functionMapOpt.value();
}