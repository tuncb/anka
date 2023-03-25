#include "tokenizer.h"

#include <cctype>
#include <optional>

auto isDigit(const char c) -> bool
{
  return (c >= '0' && c <= '9');
}

auto isNumber(const char c) -> bool
{
  return (c >= '0' && c <= '9') || c == '-' || c == '.';
}

auto isEndLine(const char c) -> bool
{
  return c == '\n' || c == '\r';
}

auto isSpace(const char c) -> bool
{
  return std::isblank(static_cast<unsigned char>(c));
}

auto isName(const char c) -> bool
{
  auto ch = static_cast<unsigned char>(c);
  return std::isalpha(ch) || std::isdigit(ch) || ch == '_';
}

template <typename Predicate>
auto parseContinuously(Predicate predicate, const std::string_view content, size_t pos) -> size_t
{
  const auto b = content.begin() + pos;
  const auto e = content.end();
  auto next = std::ranges::find_if_not(b, e, predicate);
  return std::distance(b, next);
}

auto parseUntil(const std::string_view content, size_t start, const char *ch) -> std::optional<size_t>
{
  auto pos = content.find_first_of(ch, start);
  if (pos == std::string_view::npos)
    return std::nullopt;
  return pos - start;
}

auto anka::extractTokens(const std::string_view content) -> std::vector<Token>
{
  std::vector<Token> tokens;

  constexpr const char array_start_char = '(';
  constexpr const char array_end_char = ')';
  constexpr const char tuple_start_char = '[';
  constexpr const char tuple_end_char = ']';
  constexpr const char placeholder_start_char = '_';
  constexpr const char executor_start_char = '|';
  constexpr const char executor_end_char = '|';
  constexpr const char block_start_char = '{';
  constexpr const char block_end_char = '}';

  auto needSeparator = false;
  auto addConnector = false;

  size_t i = 0;
  while (i < content.size())
  {
    const auto ch = content[i];
    if (ch == array_start_char)
    {
      tokens.push_back(Token{TokenType::ArrayStart, i, 1});
      needSeparator = false;
    }
    else if (ch == array_end_char)
    {
      tokens.push_back(Token{TokenType::ArrayEnd, i, 1});
      needSeparator = false;
    }
    else if (ch == tuple_start_char)
    {
      tokens.push_back(Token{TokenType::TupleStart, i, 1});
      needSeparator = false;
    }
    else if (ch == tuple_end_char)
    {
      tokens.push_back(Token{TokenType::TupleEnd, i, 1});
      needSeparator = false;
    }
    else if (ch == executor_start_char)
    {
      tokens.push_back(Token{TokenType::Executor, i, 1});
      needSeparator = false;
    }
    else if (ch == executor_end_char)
    {
      tokens.push_back(Token{TokenType::Executor, i, 1});
      needSeparator = false;
    }
    else if (ch == block_start_char)
    {
      tokens.push_back(Token{TokenType::BlockStart, i, 1});
      needSeparator = false;
    }
    else if (ch == block_end_char)
    {
      tokens.push_back(Token{TokenType::BlockEnd, i, 1});
      needSeparator = false;
    }
    else if (ch == placeholder_start_char)
    {
      auto len = parseContinuously(isDigit, content, i + 1);
      tokens.push_back({TokenType::Placeholder, i, len + 1});
    }
    else if (isNumber(ch))
    {
      if (needSeparator)
        throw TokenizerError{i, ch};
      auto len = parseContinuously(isNumber, content, i);
      auto str = std::string_view(content.data() + i, len);
      auto type = (str.find('.') != std::string_view::npos) ? TokenType::NumberDouble : TokenType::NumberInt;
      tokens.push_back(Token{type, i, len});
      needSeparator = true;
    }
    else if (isName(ch))
    {
      if (needSeparator)
        throw TokenizerError{i, ch};

      auto len = parseContinuously(isName, content, i);
      tokens.push_back(Token{TokenType::Name, i, len});

      // a name connected to a tuple
      if (i + len < content.size() && content[i + len] == tuple_start_char)
      {
        addConnector = true;
      }

      needSeparator = true;
    }
    else if (isEndLine(ch))
    {
      tokens.push_back(Token{TokenType::SentenceEnd, i, parseContinuously(isEndLine, content, i)});
      needSeparator = false;
    }
    else if (isSpace(ch))
    {
      i = i + parseContinuously(isSpace, content, i);
      needSeparator = false;
      continue;
    }
    else
    {
      throw TokenizerError{i, ch};
    }

    i = i + tokens.back().len;
    if (addConnector)
    {
      const auto &lastToken = tokens.back();
      tokens.push_back({TokenType::Connector, lastToken.start + lastToken.len, 0});
      addConnector = false;
    }
  }

  if (tokens.size() > 0 && tokens.back().type != TokenType::SentenceEnd)
  {
    tokens.push_back(Token{TokenType::SentenceEnd, content.size(), 0});
  }
  return tokens;
}

auto anka::toString(TokenType type) -> std::string
{
  switch (type)
  {
  case TokenType::NumberInt:
    return "integer";
  case TokenType::NumberDouble:
    return "double";
  case TokenType::Name:
    return "name";
  case TokenType::ArrayStart:
    return "array start '('";
  case TokenType::ArrayEnd:
    return "array end ')'";
  case TokenType::TupleStart:
    return "tuple start '['";
  case TokenType::TupleEnd:
    return "tuple end ']'";
  case TokenType::SentenceEnd:
    return "sentence end";
  case TokenType::Placeholder:
    return "placeholder";
  default:
    throw(std::runtime_error("Fatal error: Unexpected token type"));
  }
}
