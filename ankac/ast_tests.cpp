#ifndef DOCTEST_CONFIG_DISABLE

#include <doctest/doctest.h>

#include "ast.h"
#include "tokenizer.h"

auto toContext(std::string_view content) -> anka::Context
{
  auto tokens = anka::extractTokens(content);
  return anka::createAST(content, tokens);
}

TEST_CASE("empty content")
{
  auto context = toContext("");
  CHECK(context.sentences.empty());
  CHECK(context.integerArrays.empty());
  CHECK(context.integerNumbers.empty());
}

TEST_CASE("single array")
{
  auto context = toContext("(10 20 30)");
  REQUIRE_EQ(context.sentences.size(), 1);
  REQUIRE_EQ(context.integerArrays.size(), 1);

  const auto &arr = context.integerArrays[0];
  CHECK_EQ(arr, std::vector<int>{10, 20, 30});
}

TEST_CASE("number")
{
  auto context = toContext("10 20 30");
  REQUIRE_EQ(context.sentences.size(), 1);
  REQUIRE_EQ(context.integerNumbers.size(), 3);

  CHECK_EQ(context.integerNumbers, std::vector<int>{10, 20, 30});
}

TEST_CASE("number array combos")
{
  using namespace anka;

  auto context = toContext("40 (10 20 30) 50 (1 2 3)");
  REQUIRE_EQ(context.sentences.size(), 1);
  REQUIRE_EQ(context.integerNumbers.size(), 2);
  REQUIRE_EQ(context.integerArrays.size(), 2);

  CHECK_EQ(context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(context.integerArrays[1], std::vector<int>{1, 2, 3});

  auto &words = context.sentences[0].words;
  CHECK_EQ(words, std::vector<Word>{{WordType::IntegerNumber, 0},
                                    {WordType::IntegerArray, 0},
                                    {WordType::IntegerNumber, 1},
                                    {WordType::IntegerArray, 1}});
}

TEST_CASE("multi sentence")
{
  using namespace anka;

  auto context = toContext("40 (10 20 30)\n 50 (1 2 3)");
  REQUIRE_EQ(context.sentences.size(), 2);
  REQUIRE_EQ(context.integerNumbers.size(), 2);
  REQUIRE_EQ(context.integerArrays.size(), 2);

  CHECK_EQ(context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(context.integerArrays[1], std::vector<int>{1, 2, 3});

  CHECK_EQ(context.sentences[0].words, std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::IntegerArray, 0}});
  CHECK_EQ(context.sentences[1].words, std::vector<Word>{{WordType::IntegerNumber, 1}, {WordType::IntegerArray, 1}});
}

#endif
