#pragma once
#include <optional>
#include <string>

#include "ast.h"

namespace anka
{

struct ExecutionError
{
  std::optional<Word> word1;
  std::optional<Word> word2;

  std::string msg;
};

auto execute(Context &context, const std::vector<Sentence> &sentences) -> std::optional<Word>;
} // namespace anka