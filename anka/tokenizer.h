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
  ArrayEnd,
  Connector,
  NumberInt,
  NumberDouble,
  SentenceEnd,
  Name,
  TupleStart,
  TupleEnd,
  Placeholder,
  Executor,
};

auto toString(TokenType type) -> std::string;

struct Token
{
  TokenType type;
  size_t start = 0;
  size_t len = 0;

  auto operator<=>(const Token &) const = default;
};

auto extractTokens(const std::string_view content) -> std::vector<Token>;

} // namespace anka