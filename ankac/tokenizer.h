#pragma once
#include <string_view>
#include <vector>
#include <stdexcept>

namespace anka
{

struct TokenizerError {
  size_t pos;
  char ch;
};


enum class TokenType
{
  ArrayStart,
  NumberInt,
  ArrayEnd,
  SentenceEnd
};

struct Token
{
  TokenType type;
  size_t token_start = 0;
  size_t len = 0;
};

auto extract_tokens(const std::string_view content) -> std::vector<Token>;

} // namespace anka