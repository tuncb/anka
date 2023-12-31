#pragma once
#include <optional>
#include <string>

#include "ast.h"

namespace anka
{

auto execute(Context &context, const std::vector<Sentence> &sentences) -> std::optional<Word>;
} // namespace anka