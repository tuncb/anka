#ifndef DOCTEST_CONFIG_DISABLE

#include "tokenizer.h"
#include <doctest/doctest.h>
#include <range/v3/view/zip.hpp>

// Tests

auto checkTokens(const std::string_view text, const std::vector<anka::Token> &expected) -> void
{
  auto tokens = anka::extractTokens(text);
  for (auto test_pair : ranges::views::zip(tokens, expected))
  {
    CHECK_EQ(std::get<0>(test_pair), std::get<1>(test_pair));
  }
}

TEST_CASE("test array tokenizing")
{
  checkTokens("(1 2 3)",
              {anka::Token{anka::TokenType::ArrayStart, 0, 1}, anka::Token{anka::TokenType::NumberInt, 1, 1},
               anka::Token{anka::TokenType::NumberInt, 3, 1}, anka::Token{anka::TokenType::NumberInt, 5, 1},
               anka::Token{anka::TokenType::ArrayEnd, 6, 1}, anka::Token{anka::TokenType::SentenceEnd, 7, 0}});

  checkTokens(" (123 2  3444)",
              {anka::Token{anka::TokenType::ArrayStart, 1, 1}, anka::Token{anka::TokenType::NumberInt, 2, 3},
               anka::Token{anka::TokenType::NumberInt, 6, 1}, anka::Token{anka::TokenType::NumberInt, 9, 4},
               anka::Token{anka::TokenType::ArrayEnd, 13, 1}, anka::Token{anka::TokenType::SentenceEnd, 14, 0}});

  checkTokens("  (1 2 3)\n  (1 2 3)", {
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

TEST_CASE("test tuple tokenizing")
{
  checkTokens("[1 2 3]",
              {anka::Token{anka::TokenType::TupleStart, 0, 1}, anka::Token{anka::TokenType::NumberInt, 1, 1},
               anka::Token{anka::TokenType::NumberInt, 3, 1}, anka::Token{anka::TokenType::NumberInt, 5, 1},
               anka::Token{anka::TokenType::TupleEnd, 6, 1}, anka::Token{anka::TokenType::SentenceEnd, 7, 0}});

  checkTokens(" [123 2  3444]",
              {anka::Token{anka::TokenType::TupleStart, 1, 1}, anka::Token{anka::TokenType::NumberInt, 2, 3},
               anka::Token{anka::TokenType::NumberInt, 6, 1}, anka::Token{anka::TokenType::NumberInt, 9, 4},
               anka::Token{anka::TokenType::TupleEnd, 13, 1}, anka::Token{anka::TokenType::SentenceEnd, 14, 0}});

  checkTokens("  [1 2 3]\n  [1 2 3]", {
                                          anka::Token{anka::TokenType::TupleStart, 2, 1},
                                          anka::Token{anka::TokenType::NumberInt, 3, 1},
                                          anka::Token{anka::TokenType::NumberInt, 5, 1},
                                          anka::Token{anka::TokenType::NumberInt, 7, 1},
                                          anka::Token{anka::TokenType::TupleEnd, 8, 1},
                                          anka::Token{anka::TokenType::SentenceEnd, 9, 1},
                                          anka::Token{anka::TokenType::TupleStart, 12, 1},
                                          anka::Token{anka::TokenType::NumberInt, 13, 1},
                                          anka::Token{anka::TokenType::NumberInt, 15, 1},
                                          anka::Token{anka::TokenType::NumberInt, 17, 1},
                                          anka::Token{anka::TokenType::TupleEnd, 18, 1},
                                          anka::Token{anka::TokenType::SentenceEnd, 19, 0},
                                      });

  checkTokens("[1 (1 2 3) [4 (1 2)]]",
              {anka::Token{anka::TokenType::TupleStart, 0, 1}, anka::Token{anka::TokenType::NumberInt, 1, 1},
               anka::Token{anka::TokenType::ArrayStart, 3, 1}, anka::Token{anka::TokenType::NumberInt, 4, 1},
               anka::Token{anka::TokenType::NumberInt, 6, 1}, anka::Token{anka::TokenType::NumberInt, 8, 1},
               anka::Token{anka::TokenType::ArrayEnd, 9, 1}, anka::Token{anka::TokenType::TupleStart, 11, 1},
               anka::Token{anka::TokenType::NumberInt, 12, 1}, anka::Token{anka::TokenType::ArrayStart, 14, 1},
               anka::Token{anka::TokenType::NumberInt, 15, 1}, anka::Token{anka::TokenType::NumberInt, 17, 1},
               anka::Token{anka::TokenType::ArrayEnd, 18, 1}, anka::Token{anka::TokenType::TupleEnd, 19, 1},
               anka::Token{anka::TokenType::TupleEnd, 20, 1}, anka::Token{anka::TokenType::SentenceEnd, 21, 0}});
}

TEST_CASE("test number tokenizing")
{
  checkTokens("1 20 3",
              {anka::Token{anka::TokenType::NumberInt, 0, 1}, anka::Token{anka::TokenType::NumberInt, 2, 2},
               anka::Token{anka::TokenType::NumberInt, 5, 1}, anka::Token{anka::TokenType::SentenceEnd, 6, 0}});
}

TEST_CASE("test name")
{
  checkTokens("ioata 20\nioata   \t 5",
              {anka::Token{anka::TokenType::Name, 0, 5}, anka::Token{anka::TokenType::NumberInt, 6, 2},
               anka::Token{anka::TokenType::SentenceEnd, 8, 1}, anka::Token{anka::TokenType::Name, 9, 5},
               anka::Token{anka::TokenType::NumberInt, 19, 1}, anka::Token{anka::TokenType::SentenceEnd, 20, 0}});
}

TEST_CASE("no exceptions")
{
  CHECK_NOTHROW(anka::extractTokens("40 (10 20 30)\n 50 (1 2 3)"));
}

TEST_CASE("test tokenizing error: foreign character")
{
  CHECK_THROWS_AS(anka::extractTokens("(12 23 45t)"), const anka::TokenizerError &);
}

#endif