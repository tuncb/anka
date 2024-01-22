module;
#include <format>
#include <optional>
#include <string>
#include <vector>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/transform.hpp>

#include <fmt/ranges.h>
export module anka:state_utilities;

import :interpreter_state;
import :internal_functions;

namespace anka
{
export auto anka::getFoldableWord(const anka::Context &context, const anka::Word &word) -> std::optional<Word>
{
  using namespace anka;

  if (word.type == WordType::Assignment || word.type == WordType::PlaceHolder)
    return std::nullopt;

  if (word.type != WordType::Name)
    return word;

  const auto &name = context.names[word.index];

  if (!getInternalFunctionDefinitionsWithName(name).empty())
  {
    return word;
  }

  if (auto iter = context.userDefinedNames.find(name); iter != context.userDefinedNames.end())
    return iter->second;

  return std::nullopt;
}

auto anka::toString(const anka::Context &context, const anka::Word &word) -> std::string
{
  using namespace anka;

  switch (word.type)
  {
  case WordType::IntegerNumber:
    return std::format("{}", context.integerNumbers[word.index]);
  case WordType::IntegerArray: {
    auto &v = context.integerArrays[word.index];
    return fmt::format("({})", fmt::join(v, " "));
  }
  case WordType::DoubleNumber:
    return formatDouble(context.doubleNumbers[word.index]);
  case WordType::DoubleArray: {
    auto v = context.doubleArrays[word.index] | ranges::views::transform(formatDouble) |
             ranges::to<std::vector<std::string>>;
    return fmt::format("({})", fmt::join(v, " "));
  }
  case WordType::Boolean:
    return std::format("{}", context.booleans[word.index]);
  case WordType::Block:
    return "User defined block.";
  case WordType::BooleanArray: {
    auto &v = context.booleanArrays[word.index];
    return fmt::format("({})", fmt::join(v, " "));
  }
  case WordType::Tuple: {
    std::vector<std::string> names;
    auto &&tup = context.tuples[word.index];
    std::transform(tup.words.begin(), tup.words.end(), std::back_inserter(names),
                   [&context](const Word &word) { return toString(context, word); });
    return fmt::format("[{}]", fmt::join(names, " "));
  }
  case WordType::Name: {
    const auto &name = context.names[word.index];

    auto definitions = getInternalFunctionDefinitionsWithName(name);
    auto definitionTexts = definitions | ranges::views::transform([](const auto &def) { return anka::toString(def); }) |
                           ranges::to<std::vector<std::string>>();

    if (!definitionTexts.empty())
    {
      return fmt::format("{}", fmt::join(definitionTexts, "\n"));
    }

    return name;
  }
  default:
    return "";
  }
}
} // namespace anka