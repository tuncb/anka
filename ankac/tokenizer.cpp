#include "tokenizer.h"

#include <cctype>

auto isNumber(const char c) -> bool
{
  return (c >= '0' && c <= '9') || c == '-';
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
  return std::isalpha(ch) || std::isdigit(ch);
}

template <typename Predicate>
auto parseContinuously(Predicate predicate, const std::string_view content, size_t pos) -> size_t
{
  const auto b = content.begin() + pos;
  const auto e = content.end();
  auto next = std::ranges::find_if_not(b, e, predicate);
  return std::distance(b, next);
}

auto anka::extractTokens(const std::string_view content) -> std::vector<Token>
{
  std::vector<Token> tokens;

  constexpr const char array_start_char = '(';
  constexpr const char array_end_char = ')';
  constexpr const char tuple_start_char = '[';
  constexpr const char tuple_end_char = ']';

  auto needSeparator = false;

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
    else if (isNumber(ch))
    {
      if (needSeparator)
        throw TokenizerError{i, ch};
      tokens.push_back(Token{TokenType::NumberInt, i, parseContinuously(isNumber, content, i)});
      needSeparator = true;
    }
    else if (isName(ch))
    {
      if (needSeparator)
        throw TokenizerError{i, ch};
      tokens.push_back(Token{TokenType::Name, i, parseContinuously(isName, content, i)});
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
  }

  if (tokens.size() > 0 && tokens.back().type != TokenType::SentenceEnd)
  {
    tokens.push_back(Token{TokenType::SentenceEnd, content.size(), 0});
  }
  return tokens;
}