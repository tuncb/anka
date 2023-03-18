#pragma once
#include <string>
#include <unordered_map>

#include "ast.h"

namespace anka {

auto getInternalFunctions() -> const std::unordered_map<std::string, anka::Function>&;

}