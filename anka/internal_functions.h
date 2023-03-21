#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.h"

namespace anka
{

enum class InternalFunctionType
{
  Int__IntArray,
  Int__Int,
  Bool__Bool,
  IntArray__Int,
  IntArray__IntArray,
  Int_Int__Int,
  Int_Int_Bool,
  Bool_Bool__Bool,
  BoolArray__Int,
};

auto toString(anka::InternalFunctionType type) -> std::string;

struct InternalFunction
{
  void *ptr = nullptr;
  InternalFunctionType type;
};

auto getInternalFunctions() -> const std::unordered_map<std::string, std::vector<anka::InternalFunction>> &;

} // namespace anka