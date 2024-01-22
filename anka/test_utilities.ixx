module;
#include <vector>
export module test_utilities;

import anka;

export struct ParseResult
{
  anka::Context context;
  std::vector<anka::Sentence> sentences;
};