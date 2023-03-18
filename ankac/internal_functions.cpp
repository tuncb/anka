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

auto anka::getInternalFunctions() -> const std::unordered_map<std::string, anka::Function> &
{
  static std::optional<std::unordered_map<std::string, anka::Function>> functionMapOpt;
  if (functionMapOpt.has_value())
    return functionMapOpt.value();

  std::unordered_map<std::string, anka::Function> map;
  map["ioata"] = {&ioata, {WordType::IntegerNumber}, {WordType::IntegerArray}};

  functionMapOpt = std::move(map);
  return functionMapOpt.value();
}