#pragma once
#include <stdexcept>
#include <string_view>
#include <vector>

namespace anka
{

struct TokenizerError
{
  size_t pos;
  char ch;
};

enum class TokenType
{
  ArrayStart,
  NumberInt,
  NumberDouble,
  ArrayEnd,
  SentenceEnd,
  Name,
  TupleStart,
  TupleEnd
};

struct Token
{
  TokenType type;
  size_t token_start = 0;
  size_t len = 0;

  auto operator<=>(const Token &) const = default;
};

auto extractTokens(const std::string_view content) -> std::vector<Token>;

} // namespace anka