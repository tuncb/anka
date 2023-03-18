#pragma once
#include <optional>
#include <string>

#include "ast.h"

namespace anka
{

struct ExecutionError
{
  Context& context;
  std::optional<Word> word1;
  std::optional<Word> word2;

  std::string msg;
};

auto execute(Context &context) -> std::optional<Word>;
} // namespace anka