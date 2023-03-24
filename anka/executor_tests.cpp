#ifndef DOCTEST_CONFIG_DISABLE

#include <doctest/doctest.h>

#include <string_view>

#include "ast.h"
#include "executor.h"
#include "tokenizer.h"

TEST_CASE("empty context")
{
  anka::Context context;
  anka::AST ast{context, std::vector<anka::Sentence>{}};
  CHECK_FALSE(anka::execute(ast).has_value());
}

auto executeText(const std::string_view content) -> std::string
{
  anka::Context context;
  auto tokens = anka::extractTokens(content);
  auto ast = anka::parseAST(content, tokens, std::move(context));
  auto res = anka::execute(ast);
  if (res.has_value())
  {
    return anka::toString(ast.context, res.value());
  }

  return "";
}

TEST_CASE("single values")
{
  CHECK_EQ(executeText("(1 2 3)"), "(1 2 3)");
  CHECK_EQ(executeText("(10 20 30) \n(1 2 3)"), "(1 2 3)");
  CHECK_EQ(executeText("1"), "1");
  CHECK_EQ(executeText("1  \n  2"), "2");
}

TEST_CASE("internal functions")
{
  CHECK_EQ(executeText("ioata 5"), "(1 2 3 4 5)");
  CHECK_THROWS_AS(executeText("ioata (1 2 3)"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("equals 1"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("add [1]"), const anka::ExecutionError &);
  CHECK_EQ(executeText("inc 5"), "6");
  CHECK_EQ(executeText("inc (1 2 3)"), "(2 3 4)");
  CHECK_EQ(executeText("dec (1 2 3)"), "(0 1 2)");
  CHECK_EQ(executeText("dec inc 5"), "5");
  CHECK_EQ(executeText("dec inc (7 8 9)"), "(7 8 9)");
  CHECK_EQ(executeText("ioata inc inc 5"), "(1 2 3 4 5 6 7)");
}

TEST_CASE("internal function overloads")
{
  CHECK_EQ(executeText("equals [1 1]"), "true");
  CHECK_EQ(executeText("equals [true true]"), "true");
  CHECK_EQ(executeText("equals [true (false true)]"), "(false true)");
  CHECK_EQ(executeText("equals [(1 2 3) (1 3 4)]"), "(true false false)");
  CHECK_EQ(executeText("inc 1.0"), "2.0");
  CHECK_EQ(executeText("dec [1.0]"), "0.0");
  CHECK_EQ(executeText("add [3.0 1.0]"), "4.0");
  CHECK_EQ(executeText("sub [3.0 1.0]"), "2.0");
  CHECK_EQ(executeText("mul [3.0 1.0]"), "3.0");
  CHECK_EQ(executeText("div [3.0 1.0]"), "3.0");
  CHECK_EQ(executeText("sort (3.0 4.0 1.0)"), "(1.0 3.0 4.0)");
  CHECK_EQ(executeText("length (3.0 4.0 1.0)"), "3");
}

TEST_CASE("single tuple arguments")
{
  CHECK_EQ(executeText("ioata [5]"), "(1 2 3 4 5)");
  CHECK_THROWS_AS(executeText("ioata [(1 2 3)]"), const anka::ExecutionError &);
  CHECK_EQ(executeText("inc [5]"), "6");
  CHECK_EQ(executeText("inc [(1 2 3)]"), "(2 3 4)");
  CHECK_EQ(executeText("dec [(1 2 3)]"), "(0 1 2)");
  CHECK_EQ(executeText("dec inc [5]"), "5");
  CHECK_EQ(executeText("dec inc [(7 8 9)]"), "(7 8 9)");
  CHECK_EQ(executeText("ioata inc inc [5]"), "(1 2 3 4 5 6 7)");
}

TEST_CASE("double argument rank polymorphism")
{
  CHECK_EQ(executeText("add [5 6]"), "11");
  CHECK_EQ(executeText("add [(1 2 3) (3 4 5)]"), "(4 6 8)");
  CHECK_THROWS_AS(executeText("add [(1 2 3 4) (3 4 5)]"), const anka::ExecutionError &);
  CHECK_EQ(executeText("mul [10 (3 4 5)]"), "(30 40 50)");
  CHECK_EQ(executeText("sub [(3 4 5) 1]"), "(2 3 4)");
}

TEST_CASE("mismatch errors")
{
  CHECK_THROWS_AS(executeText("10 20"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("10 (20 40)"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("(10) (20 40)"), const anka::ExecutionError &);
}

TEST_CASE("connected tuples")
{
  CHECK_EQ(executeText("add[10 _] 20"), "30");
  CHECK_EQ(executeText("add [10 _1] [20]"), "30");
  CHECK_EQ(executeText("add [_] [10 20]"), "30");
  CHECK_THROWS_AS(executeText("add [10] 20"), const anka::ExecutionError &);
}

#endif