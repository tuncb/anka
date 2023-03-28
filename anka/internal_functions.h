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
};

auto toString(anka::InternalFunctionType type) -> std::string;

struct InternalFunction
{
  void *ptr = nullptr;
  InternalFunctionType type;
};

auto getInternalFunctions() -> const std::unordered_map<std::string, std::vector<anka::InternalFunction>> &;

auto getInternalDoubleConstants() -> const std::unordered_map<std::string, double> &;

template <typename T> auto getInternalConstants() -> const std::unordered_map<std::string, T> &
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

} // namespace anka