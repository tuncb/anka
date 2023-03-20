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

TEST_CASE("simple tuple")
{
  using namespace anka;

  auto ast = toAST("[1 2 3]");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 3);
  REQUIRE_EQ(ast.context.tuples.size(), 1);

  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{1, 2, 3});

  CHECK_EQ(ast.context.tuples[0],
           std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::IntegerNumber, 1}, {WordType::IntegerNumber, 2}});
  CHECK_EQ(ast.sentences[0].words, std::vector<Word>{{WordType::Tuple, 0}});
}

TEST_CASE("complex tuple")
{
  using namespace anka;

  auto ast = toAST("[1 (1 2) [3 4] 11]");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 4);
  REQUIRE_EQ(ast.context.tuples.size(), 2);
  REQUIRE_EQ(ast.context.integerArrays.size(), 1);

  CHECK_EQ(ast.context.integerArrays[0], std::vector<int>{1, 2});
  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{1, 3, 4, 11});

  CHECK_EQ(ast.context.tuples[0], std::vector<Word>{{WordType::IntegerNumber, 1}, {WordType::IntegerNumber, 2}});
  CHECK_EQ(ast.context.tuples[1], std::vector<Word>{{WordType::IntegerNumber, 0},
                                                    {WordType::IntegerArray, 0},
                                                    {WordType::Tuple, 0},
                                                    {WordType::IntegerNumber, 3}});
  CHECK_EQ(ast.sentences[0].words, std::vector<Word>{{WordType::Tuple, 1}});
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

TEST_CASE("booleans")
{
  using namespace anka;

  auto ast = toAST("true false");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.booleans.size(), 2);
  CHECK_EQ(ast.context.booleans, std::vector<bool>{true, false});
  CHECK_EQ(ast.sentences[0].words, std::vector<Word>{{WordType::Boolean, 0}, {WordType::Boolean, 1}});
}

TEST_CASE("boolean array")
{
  using namespace anka;

  auto ast = toAST("(true false)");
  REQUIRE_EQ(ast.sentences.size(), 1);
  REQUIRE_EQ(ast.context.booleanArrays.size(), 1);
  CHECK_EQ(ast.context.booleanArrays[0], std::vector<bool>{true, false});
  CHECK_EQ(ast.sentences[0].words, std::vector<Word>{{WordType::BooleanArray, 0}});
}

TEST_CASE("names")
{
  using namespace anka;

  auto ast = toAST("add ioata (10 20 30)\n ioata 50 (1 2 3)");
  REQUIRE_EQ(ast.sentences.size(), 2);
  REQUIRE_EQ(ast.context.integerNumbers.size(), 1);
  REQUIRE_EQ(ast.context.integerArrays.size(), 2);
  REQUIRE_EQ(ast.context.names.size(), 3);

  CHECK_EQ(ast.context.integerNumbers, std::vector<int>{50});
  CHECK_EQ(ast.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ast.context.integerArrays[1], std::vector<int>{1, 2, 3});
  CHECK_EQ(ast.context.names, std::vector<std::string>{"add", "ioata", "ioata"});

  CHECK_EQ(ast.sentences[0].words,
           std::vector<Word>{{WordType::Name, 0}, {WordType::Name, 1}, {WordType::IntegerArray, 0}});
  CHECK_EQ(ast.sentences[1].words,
           std::vector<Word>{{WordType::Name, 2}, {WordType::IntegerNumber, 0}, {WordType::IntegerArray, 1}});
}

#endif
