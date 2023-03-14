#include "tokenizer.h"
#include <doctest/doctest.h>

#include <range/v3/view/zip.hpp>

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
  while (i < content.size())
  {
    if (!predicate(content[i]))
      break;
    ++i;
  }
  return i - pos;
}

auto anka::extractTokens(const std::string_view content) -> std::vector<Token>
{
  std::vector<Token> tokens;

  constexpr const char array_start_char = '(';
  constexpr const char array_end_char = ')';

  size_t i = 0;
  while (i < content.size())
  {
    const auto ch = content[i];
    if (ch == array_start_char)
    {
      tokens.push_back(Token{TokenType::ArrayStart, i, 1});
    }
    else if (ch == array_end_char)
    {
      tokens.push_back(Token{TokenType::ArrayEnd, i, 1});
    }
    else if (isNumber(ch))
    {
      tokens.push_back(Token{TokenType::NumberInt, i, parseContinuously(isNumber, content, i)});
    }
    else if (isEndLine(ch))
    {
      tokens.push_back(Token{TokenType::SentenceEnd, i, parseContinuously(isEndLine, content, i)});
    }
    else if (isSpace(ch))
    {
      i = i + parseContinuously(isSpace, content, i);
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

// Tests

auto check_tokens(const std::string_view text, const std::vector<anka::Token> &expected) -> void
{
  auto tokens = anka::extractTokens(text);
  for (auto test_pair : ranges::views::zip(tokens, expected))
  {
    CHECK_EQ(std::get<0>(test_pair), std::get<1>(test_pair));
  }
}

TEST_CASE("test array tokenizing")
{
  check_tokens("(1 2 3)",
               {anka::Token{anka::TokenType::ArrayStart, 0, 1}, anka::Token{anka::TokenType::NumberInt, 1, 1},
                anka::Token{anka::TokenType::NumberInt, 3, 1}, anka::Token{anka::TokenType::NumberInt, 5, 1},
                anka::Token{anka::TokenType::ArrayEnd, 6, 1}, anka::Token{anka::TokenType::SentenceEnd, 7, 0}});

  check_tokens(" (123 2  3444)",
               {anka::Token{anka::TokenType::ArrayStart, 1, 1}, anka::Token{anka::TokenType::NumberInt, 2, 3},
                anka::Token{anka::TokenType::NumberInt, 6, 1}, anka::Token{anka::TokenType::NumberInt, 9, 4},
                anka::Token{anka::TokenType::ArrayEnd, 13, 1}, anka::Token{anka::TokenType::SentenceEnd, 14, 0}});

  check_tokens("  (1 2 3)\n  (1 2 3)", {
                                           anka::Token{anka::TokenType::ArrayStart, 2, 1},
                                           anka::Token{anka::TokenType::NumberInt, 3, 1},
                                           anka::Token{anka::TokenType::NumberInt, 5, 1},
                                           anka::Token{anka::TokenType::NumberInt, 7, 1},
                                           anka::Token{anka::TokenType::ArrayEnd, 8, 1},
                                           anka::Token{anka::TokenType::SentenceEnd, 9, 1},
                                           anka::Token{anka::TokenType::ArrayStart, 12, 1},
                                           anka::Token{anka::TokenType::NumberInt, 13, 1},
                                           anka::Token{anka::TokenType::NumberInt, 15, 1},
                                           anka::Token{anka::TokenType::NumberInt, 17, 1},
                                           anka::Token{anka::TokenType::ArrayEnd, 18, 1},
                                           anka::Token{anka::TokenType::SentenceEnd, 19, 0},
                                       });
}
