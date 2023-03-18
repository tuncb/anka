#include "executor.h"

#include <numeric>

#include <range/v3/view/reverse.hpp>
#include <range/v3/view/drop.hpp>

auto fold(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  throw anka::ExecutionError{context, w1, w2, "Could not fold words."};
}

auto anka::execute(Context &context) -> std::optional<Word>
{
  using namespace ranges;

  if (context.sentences.empty())
    return std::nullopt;

  std::optional<Word> word = std::nullopt;
  for (const auto &sentence : context.sentences)
  {
    if (sentence.words.empty())
      continue;

    auto init = sentence.words.back();
    auto rv = sentence.words | views::reverse | views::drop(1);

    word = std::accumulate(rv.begin(), rv.end(), init,
                           [&context](const Word &w1, const Word &w2) { return fold(context, w2, w1); });
  }

  return word;
}

