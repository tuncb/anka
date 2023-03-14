#include "tokenizer.h"

auto isNumber(const char c) -> bool
{
  return c >= '0' && c <= '9';
}

auto isEndLine(const char c) -> bool
{
  return c == '\n' || c == '\r';
}

auto isSpace(const char c) -> bool
{
  return c == ' ' || c == '\t';
}

template <typename Predicate>
auto parseContinuously(Predicate predicate, const std::string_view content, size_t pos) -> size_t
{
  auto i = pos + 1;
  size_t len = 1;
  while (i < content.size())
  {
    if (!predicate(content[i]))
      break;
    ++len;
  }
  return len;
}

auto anka::extract_tokens(const std::string_view content) -> std::vector<Token>
{
  std::vector<Token> tokens;
  size_t token_start = 0;
  size_t token_end = 0;

  constexpr const char array_start_char = '(';
  constexpr const char array_end_char = ')';

  size_t i = 0;
  while (i < content.size())
  {
    const auto ch = content[i];
    if (content[i] == array_start_char)
    {
      tokens.push_back(Token{TokenType::ArrayEnd, i, 1});
    }
    else if (content[i] == array_end_char)
    {
      tokens.push_back(Token{TokenType::ArrayEnd, i, 1});
    }
    else if (isNumber(content[i]))
    {
      tokens.push_back(Token{TokenType::NumberInt, i, parseContinuously(isNumber, content, i)});
    }
    else if (isEndLine(content[i]))
    {
      tokens.push_back(Token{TokenType::SentenceEnd, i, parseContinuously(isEndLine, content, i)});
    }
    else if (isSpace(content[i]))
    {
      i = i + parseContinuously(isSpace, content, i);
      continue;
    }
    else
    {
      throw TokenizerError{i, content[i]};
    }

    i = i + tokens.back().len;
  }

  if (tokens.size() > 0 && tokens.back().type != TokenType::SentenceEnd) {
    tokens.push_back(Token{TokenType::SentenceEnd, content.size(), 0});
  }
  return tokens;
}
