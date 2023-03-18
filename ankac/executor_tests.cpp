#ifndef DOCTEST_CONFIG_DISABLE

#include <doctest/doctest.h>

#include <string_view>

#include "ast.h"
#include "executor.h"
#include "tokenizer.h"

TEST_CASE("empty context")
{
  anka::Context context;
  CHECK_FALSE(anka::execute(context).has_value());
}

auto executeText(const std::string_view content) -> std::string
{
  auto tokens = anka::extractTokens(content);
  auto context = anka::createAST(content, tokens);
  auto res = anka::execute(context);
  if (res.has_value())
  {
    return anka::toString(context, res.value());
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

TEST_CASE("mismatch errors")
{
  CHECK_THROWS_AS(executeText("10 20"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("10 (20 40)"), const anka::ExecutionError &);
  CHECK_THROWS_AS(executeText("(10) (20 40)"), const anka::ExecutionError &);
}


#endif