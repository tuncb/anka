#include "executor.h"

#include <numeric>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>

auto fold(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  throw anka::ExecutionError{w1, w2, "Could not fold words."};
}

auto anka::execute(AST &ast) -> std::optional<Word>
{
  using namespace ranges;

  if (ast.sentences.empty())
    return std::nullopt;

  std::optional<Word> word = std::nullopt;
  for (const auto &sentence : ast.sentences)
  {
    if (sentence.words.empty())
      continue;

    auto init = sentence.words.back();
    auto rv = sentence.words | views::reverse | views::drop(1);

    word = std::accumulate(rv.begin(), rv.end(), init,
                           [&ast](const Word &w1, const Word &w2) { return fold(ast.context, w1, w2); });
  }

  return word;
}
