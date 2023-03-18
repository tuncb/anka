#ifndef DOCTEST_CONFIG_DISABLE

#include <doctest/doctest.h>

#include "ast.h"
#include "tokenizer.h"

auto toAST(std::string_view content) -> anka::AST
{
  anka::Context context;
  auto tokens = anka::extractTokens(content);
  return anka::parseAST(content, tokens, std::move(context));
}

TEST_CASE("empty content")
{
  auto ast = toAST("");
  CHECK(ast.sentences.empty());
  CHECK(ast.context.integerArrays.empty());
  CHECK(ast.context.integerNumbers.empty());
}

TEST_CASE("single array")
{
  auto ast = toAST("(10 20 30)");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.integerArrays.size(), 1);

  const auto &arr = ast.context.integerArrays[0];
  CHECK_EQ(arr, std::vector<int>{10, 20, 30});
}

TEST_CASE("number")
{
  auto ast = toAST("10 20 30");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 3);

  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{10, 20, 30});
}

TEST_CASE("number array combos")
{
  using namespace anka;

  auto ast = toAST("40 (10 20 30) 50 (1 2 3)");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 2);
  REQUIRE_EQ(ast.context.integerArrays.size(), 2);

  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(ast.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ast.context.integerArrays[1], std::vector<int>{1, 2, 3});

  auto &words = ast.sentences[0].words;
  CHECK_EQ(words, std::vector<Word>{{WordType::IntegerNumber, 0},
                                    {WordType::IntegerArray, 0},
                                    {WordType::IntegerNumber, 1},
                                    {WordType::IntegerArray, 1}});
}

TEST_CASE("multi sentence")
{
  using namespace anka;

  auto ast = toAST("40 (10 20 30)\n 50 (1 2 3)");
  REQUIRE_EQ(ast.sentences.size(), 2);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 2);
  REQUIRE_EQ(ast.context.integerArrays.size(), 2);

  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(ast.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ast.context.integerArrays[1], std::vector<int>{1, 2, 3});

  CHECK_EQ(ast.sentences[0].words, std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::IntegerArray, 0}});
  CHECK_EQ(ast.sentences[1].words, std::vector<Word>{{WordType::IntegerNumber, 1}, {WordType::IntegerArray, 1}});
}

#endif
