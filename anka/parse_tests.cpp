#ifndef DOCTEST_CONFIG_DISABLE

#include <doctest/doctest.h>
#include <string_view>
#include <vector>

import anka;
import test_utilities;

auto toParseResult(std::string_view content) -> ParseResult
{
  anka::Context context;
  auto tokens = anka::extractTokens(content);
  auto sentences = anka::parse(content, tokens, context);
  return {context, sentences};
}

TEST_CASE("empty content")
{
  auto ParseResult = toParseResult("");
  CHECK(ParseResult.sentences.empty());
  CHECK(ParseResult.context.integerArrays.empty());
  CHECK(ParseResult.context.integerNumbers.empty());
}

TEST_CASE("single array")
{
  auto ParseResult = toParseResult("(10 20 30)");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerArrays.size(), 1);

  const auto &arr = ParseResult.context.integerArrays[0];
  CHECK_EQ(arr, std::vector<int>{10, 20, 30});
}

TEST_CASE("number")
{
  auto ParseResult = toParseResult("10 20 30");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 3);

  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{10, 20, 30});
}

TEST_CASE("number array combos")
{
  using namespace anka;

  auto ParseResult = toParseResult("40 (10 20 30) 50 (1 2 3)");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 2);
  REQUIRE_EQ(ParseResult.context.integerArrays.size(), 2);

  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(ParseResult.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ParseResult.context.integerArrays[1], std::vector<int>{1, 2, 3});

  auto &words = ParseResult.sentences[0].words;
  CHECK_EQ(words, std::vector<Word>{{WordType::IntegerNumber, 0},
                                    {WordType::IntegerArray, 0},
                                    {WordType::IntegerNumber, 1},
                                    {WordType::IntegerArray, 1}});
}

TEST_CASE("simple tuple")
{
  using namespace anka;

  auto ParseResult = toParseResult("[1 2 3]");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 3);
  REQUIRE_EQ(ParseResult.context.tuples.size(), 1);

  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{1, 2, 3});

  CHECK_EQ(ParseResult.context.tuples[0].words,
           std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::IntegerNumber, 1}, {WordType::IntegerNumber, 2}});
  CHECK_FALSE(ParseResult.context.tuples[0].connectedNameIndexOpt);
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::Tuple, 0}});
}

TEST_CASE("complex tuple")
{
  using namespace anka;

  auto ParseResult = toParseResult("[1 (1 2) [3 4] 11]");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 4);
  REQUIRE_EQ(ParseResult.context.tuples.size(), 2);
  REQUIRE_EQ(ParseResult.context.integerArrays.size(), 1);

  CHECK_EQ(ParseResult.context.integerArrays[0], std::vector<int>{1, 2});
  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{1, 3, 4, 11});

  CHECK_EQ(ParseResult.context.tuples[0].words, std::vector<Word>{{WordType::IntegerNumber, 1}, {WordType::IntegerNumber, 2}});
  CHECK_EQ(ParseResult.context.tuples[1].words, std::vector<Word>{{WordType::IntegerNumber, 0},
                                                          {WordType::IntegerArray, 0},
                                                          {WordType::Tuple, 0},
                                                          {WordType::IntegerNumber, 3}});
  CHECK_FALSE(ParseResult.context.tuples[0].connectedNameIndexOpt);
  CHECK_FALSE(ParseResult.context.tuples[1].connectedNameIndexOpt);
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::Tuple, 1}});
}

TEST_CASE("multi sentence")
{
  using namespace anka;

  auto ParseResult = toParseResult("40 (10 20 30)\n 50 (1 2 3)");
  REQUIRE_EQ(ParseResult.sentences.size(), 2);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 2);
  REQUIRE_EQ(ParseResult.context.integerArrays.size(), 2);

  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{40, 50});
  CHECK_EQ(ParseResult.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ParseResult.context.integerArrays[1], std::vector<int>{1, 2, 3});

  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::IntegerArray, 0}});
  CHECK_EQ(ParseResult.sentences[1].words, std::vector<Word>{{WordType::IntegerNumber, 1}, {WordType::IntegerArray, 1}});
}

TEST_CASE("booleans")
{
  using namespace anka;

  auto ParseResult = toParseResult("true false");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.booleans.size(), 2);
  CHECK_EQ(ParseResult.context.booleans, std::vector<bool>{true, false});
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::Boolean, 0}, {WordType::Boolean, 1}});
}

TEST_CASE("doubles")
{
  using namespace anka;

  auto ParseResult = toParseResult("1.0 2.0");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.doubleNumbers.size(), 2);
  CHECK_EQ(ParseResult.context.doubleNumbers, std::vector<double>{1.0, 2.0});
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::DoubleNumber, 0}, {WordType::DoubleNumber, 1}});
}

TEST_CASE("boolean array")
{
  using namespace anka;

  auto ParseResult = toParseResult("(true false)");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.booleanArrays.size(), 1);
  CHECK_EQ(ParseResult.context.booleanArrays[0], std::vector<bool>{true, false});
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::BooleanArray, 0}});
}

TEST_CASE("double array")
{
  using namespace anka;

  auto ParseResult = toParseResult("(1.0 2.0)");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.doubleArrays.size(), 1);
  CHECK_EQ(ParseResult.context.doubleArrays[0], std::vector<double>{1.0, 2.0});
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::DoubleArray, 0}});
}

TEST_CASE("placeholders")
{
  using namespace anka;

  auto ParseResult = toParseResult("[_ _1 _2]");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  CHECK_EQ(ParseResult.sentences[0].words, std::vector<Word>{{WordType::Tuple, 0}});
  auto &&words = getValue<const Tuple &>(ParseResult.context, 0).words;
  CHECK_EQ(words, std::vector<Word>{
                      {WordType::PlaceHolder, 0},
                      {WordType::PlaceHolder, 1},
                      {WordType::PlaceHolder, 2},
                  });
}

TEST_CASE("names")
{
  using namespace anka;

  auto ParseResult = toParseResult("add ioata (10 20 30)\n ioata 50 (1 2 3)");
  REQUIRE_EQ(ParseResult.sentences.size(), 2);
  REQUIRE_EQ(ParseResult.context.integerNumbers.size(), 1);
  REQUIRE_EQ(ParseResult.context.integerArrays.size(), 2);
  REQUIRE_EQ(ParseResult.context.names.size(), 3);

  CHECK_EQ(ParseResult.context.integerNumbers, std::vector<int>{50});
  CHECK_EQ(ParseResult.context.integerArrays[0], std::vector<int>{10, 20, 30});
  CHECK_EQ(ParseResult.context.integerArrays[1], std::vector<int>{1, 2, 3});
  CHECK_EQ(ParseResult.context.names, std::vector<std::string>{"add", "ioata", "ioata"});

  CHECK_EQ(ParseResult.sentences[0].words,
           std::vector<Word>{{WordType::Name, 0}, {WordType::Name, 1}, {WordType::IntegerArray, 0}});
  CHECK_EQ(ParseResult.sentences[1].words,
           std::vector<Word>{{WordType::Name, 2}, {WordType::IntegerNumber, 0}, {WordType::IntegerArray, 1}});
}

TEST_CASE("executor")
{
  using namespace anka;

  auto ParseResult = toParseResult("|length add[10 _] _1|");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.executors.size(), 1);
  REQUIRE_EQ(
      ParseResult.context.executors[0].words,
      std::vector<Word>{{WordType::Name, 0}, {WordType::Name, 1}, {WordType::Tuple, 0}, {WordType::PlaceHolder, 1}});
  REQUIRE_EQ(ParseResult.context.tuples.size(), 1);
  REQUIRE(ParseResult.context.tuples[0].connectedNameIndexOpt);
  CHECK_EQ(ParseResult.context.tuples[0].connectedNameIndexOpt.value(), 1);
  REQUIRE_EQ(ParseResult.context.tuples[0].words, std::vector<Word>{{WordType::IntegerNumber, 0}, {WordType::PlaceHolder, 0}});
}

TEST_CASE("assignment")
{
  using namespace anka;

  auto ParseResult = toParseResult("inc2: {inc inc}");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.blocks.size(), 1);
  REQUIRE_EQ(ParseResult.sentences[0].words,
             std::vector<Word>{{WordType::Name, 0}, {WordType::Assignment, 0}, {WordType::Block, 0}});
}

TEST_CASE("block")
{
  using namespace anka;

  auto ParseResult = toParseResult("{div |{to_double sum} length| (1 2 3 4 5 6)}");
  REQUIRE_EQ(ParseResult.sentences.size(), 1);
  REQUIRE_EQ(ParseResult.context.blocks.size(), 2);
  REQUIRE_EQ(ParseResult.context.blocks[0].words, std::vector<Word>{{WordType::Name, 1}, {WordType::Name, 2}});
  REQUIRE_EQ(ParseResult.context.blocks[1].words,
             std::vector<Word>{{WordType::Name, 0}, {WordType::Executor, 0}, {WordType::IntegerArray, 0}});
}

#endif
