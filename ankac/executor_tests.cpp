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

TEST_CASE("ioata")
{
  CHECK_EQ(executeText("ioata 5"), "(1 2 3 4 5)");
  CHECK_THROWS_AS(executeText("ioata (1 2 3)"), const anka::ExecutionError &);
}

TEST_CASE("mismatch errors")
{
  CHECK_THROWS_AS(executeText("10 20"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("10 (20 40)"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("(10) (20 40)"), const anka::ExecutionError &);
}

#endif