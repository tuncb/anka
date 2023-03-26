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

template <typename Predicate> auto parseUntil(Predicate predicate, const std::string_view content, size_t pos) -> size_t
{
  const auto b = content.begin() + pos;
  const auto e = content.end();
  auto next = std::ranges::find_if(b, e, predicate);
  return std::distance(b, next);
}

auto anka::extractTokens(const std::string_view content) -> std::vector<Token>
{
  std::vector<Token> tokens;

  constexpr const char array_start_char = '(';
  constexpr const char array_end_char = ')';
  constexpr const char tuple_start_char = '[';
  constexpr const char tuple_end_char = ']';
  constexpr const char placeholder_start_char = '_';
  constexpr const char executor_char = '|';
  constexpr const char block_start_char = '{';
  constexpr const char block_end_char = '}';
  constexpr const char assignment_char = ':';
  constexpr const char comment_char = '#';

  auto needSeparator = false;
  auto addConnector = false;

  auto singleCharTokens = std::vector<std::pair<const char, TokenType>>{
      {array_start_char, TokenType::ArrayStart}, {array_end_char, TokenType::ArrayEnd},
      {tuple_start_char, TokenType::TupleStart}, {tuple_end_char, TokenType::TupleEnd},
      {executor_char, TokenType::Executor},      {block_start_char, TokenType::BlockStart},
      {block_end_char, TokenType::BlockEnd},     {assignment_char, TokenType::Assignment},
  };

  size_t i = 0;
  while (i < content.size())
  {
    const auto ch = content[i];

    if (auto iter = std::ranges::find(singleCharTokens, ch, &std::pair<const char, TokenType>::first);
        iter != singleCharTokens.end())
    {
      tokens.push_back(Token{iter->second, i, 1});
      needSeparator = false;
    }
    else if (ch == comment_char)
    {
      auto len = parseUntil(isEndLine, content, i);
      if (len == std::string_view::npos)
        return tokens;
      i += len - 1;
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
  case TokenType::Executor:
    return "executor '|'";
  case TokenType::BlockStart:
    return "block start '{'";
  case TokenType::BlockEnd:
    return "block end '}'";
  case TokenType::Assignment:
    return "assignment ':'";
  default:
    throw(std::runtime_error("Fatal error: Unexpected token type"));
  }
}
