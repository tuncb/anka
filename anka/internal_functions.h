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
  Int_Int__Int,
  Int_Int_Bool,
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
};

auto toString(anka::InternalFunctionType type) -> std::string;

struct InternalFunction
{
  void *ptr = nullptr;
  InternalFunctionType type;
};

auto getInternalFunctions() -> const std::unordered_map<std::string, std::vector<anka::InternalFunction>> &;

} // namespace anka