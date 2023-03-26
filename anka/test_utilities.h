#pragma once

#include <vector>

#include "ast.h"

struct AST
{
  anka::Context context;
  std::vector<anka::Sentence> sentences;
};
