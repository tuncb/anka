#include "executor.h"

#include <algorithm>
#include <format>
#include <numeric>
#include <variant>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>

import anka;

// forward declerations
auto executeWords(anka::Context &context, const std::vector<anka::Word> &words) -> std::optional<anka::Word>;

auto getArrayItemType(anka::WordType arrType) -> std::optional<anka::WordType>
{
  switch (arrType)
  {
  case anka::WordType::IntegerArray:
    return anka::WordType::IntegerNumber;
  case anka::WordType::DoubleArray:
    return anka::WordType::DoubleNumber;
  case anka::WordType::BooleanArray:
    return anka::WordType::Boolean;
  default:
    return std::nullopt;
  }
}

struct Interpretation
{
  std::vector<anka::TypeVariant> arguments;
  std::vector<bool> expandArray;
};

template <typename Fun> auto addInterpretation(std::vector<Interpretation> &allPossibilities, Fun fun)
{
  std::vector<Interpretation> newOnes;
  for (auto possibility : allPossibilities)
  {
    auto newPossibility = fun(possibility);
    newOnes.push_back(newPossibility);
  }
  allPossibilities.insert(allPossibilities.end(), newOnes.begin(), newOnes.end());
}

auto toType(const std::vector<anka::WordType> &wtype) -> std::vector<anka::TypeVariant>
{
  return wtype | ranges::views::transform([](const auto w) { return anka::toType(w); }) |
         ranges::to<std::vector<anka::TypeVariant>>;
}

auto toFunction(const anka::InternalFunctionDefinition &def) -> anka::FunctionType
{
  anka::FunctionType ret;
  ret.returnType = anka::TypeFamily::Void;
  for (auto t : def.argumentTypes)
  {
    ret.arguments.push_back(std::get<anka::TypeFamily>(t));
  }
  return ret;
}

auto getAllInterpretations(const anka::Context &context, const std::vector<anka::Word> &words)
    -> std::vector<Interpretation>
{
  const auto wordTypes = anka::getWordTypes(words);

  std::vector<Interpretation> allPossibilities;
  allPossibilities.push_back({toType(wordTypes), std::vector<bool>(wordTypes.size())});

  for (auto [i, type] : ranges::views::enumerate(wordTypes))
  {
    const auto itemType = getArrayItemType(type);
    if (itemType)
    {
      addInterpretation(allPossibilities, [i, &itemType](const Interpretation &possibility) {
        auto newPossibility = possibility;
        newPossibility.arguments[i] = anka::toType(itemType.value());
        newPossibility.expandArray[i] = true;
        return newPossibility;
      });
    }

    if (type == anka::WordType::IntegerNumber)
    {
      addInterpretation(allPossibilities, [i, &itemType](const Interpretation &possibility) {
        auto newPossibility = possibility;
        newPossibility.arguments[i] = anka::toType(anka::WordType::DoubleNumber);
        return newPossibility;
      });
    }
    else if (type == anka::WordType::Name)
    {
      auto word = words[i];
      auto name = anka::getValue<std::string>(context, word.index);
      auto allInternalFunctionWithName = anka::getInternalFunctionDefinitionsWithName(name);
      for (auto &def : allInternalFunctionWithName)
      {
        addInterpretation(allPossibilities, [i, &itemType, &def](const Interpretation &possibility) {
          auto newPossibility = possibility;
          auto func = toFunction(def);
          newPossibility.arguments[i] = func;
          return newPossibility;
        });
      }
    }
  }

  return allPossibilities;
}

struct ExecutionInformation
{
  anka::InternalFunctionExecuter executer;
  Interpretation interpretation;
  std::vector<anka::Word> allWords;
};

auto replaceUserDefinedNames(const anka::Context &context, const std::vector<anka::Word> &words)
    -> std::vector<anka::Word>
{
  auto replaced = words;
  for (auto [i, w] : ranges::views::enumerate(replaced))
  {
    if (w.type == anka::WordType::Name)
    {
      auto name = anka::getValue<std::string>(context, w.index);
      auto iter = context.userDefinedNames.find(name);
      if (iter != context.userDefinedNames.end())
      {
        replaced[i] = iter->second;
      }
    }
  }

  return replaced;
}

auto findOverload(const anka::Context &context, const std::string &name, const anka::Word &word)
    -> std::optional<ExecutionInformation>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  const auto allWords = replaceUserDefinedNames(context, anka::getAllWords(context, word));

  const auto interpretations = getAllInterpretations(context, allWords);

  for (auto interpretation : interpretations)
  {
    // anka::WordType::Name is a dummy => fix this!!!, do we really need the result type in the definition?
    const auto definition =
        anka::InternalFunctionDefinition{name, interpretation.arguments, anka::TypeFamily::Void, nullptr};
    auto iter = internalFunctions.find(definition);
    if (iter != internalFunctions.end())
    {
      auto isExpanding = std::ranges::any_of(interpretation.expandArray, [](bool v) { return v; });

      // we don't support array of arrays
      if (isExpanding && !anka::isExpandable(iter->first.returnType))
        continue;

      return ExecutionInformation{iter->second, interpretation, allWords};
    }
  }

  return std::nullopt;
}

auto foldPlaceholder(anka::Context &context, const anka::Word &placeholder, const anka::Word &rhs) -> anka::Word
{
  using namespace anka;

  if (rhs.type == WordType::PlaceHolder)
    throw anka::ExecutionError{placeholder, rhs, "Cannot fold a placeholder to another."};

  if (rhs.type != WordType::Tuple)
  {
    if (placeholder.index == 1)
      return rhs;
    throw anka::ExecutionError{placeholder, rhs, std::format("Placeholder _{} is out of range.", placeholder.index)};
  }

  auto &&tup = getValue<const Tuple &>(context, rhs.index);
  auto idx = placeholder.index - 1;
  if (idx >= 0 && idx < tup.words.size())
  {
    return tup.words[idx];
  }

  throw anka::ExecutionError{placeholder, rhs, std::format("Placeholder _{} is out of range.", placeholder.index)};
}

auto foldtuple(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  auto &&tup = anka::getValue<const anka::Tuple &>(context, w2.index);

  using namespace anka;
  std::vector<Word> words;
  auto wordsToBeInserted = getAllWords(context, w1);

  for (auto &&w : tup.words)
  {
    if (w.type == WordType::PlaceHolder)
    {
      if (w.index == 0)
      {
        words.insert(words.end(), wordsToBeInserted.begin(), wordsToBeInserted.end());
      }
      else
      {
        auto idx = w.index - 1;
        if (idx < 0 || idx >= wordsToBeInserted.size())
          throw anka::ExecutionError{w1, std::nullopt, std::format("Placeholder {} is out of range.", w.index)};
        words.push_back(wordsToBeInserted[idx]);
      }
    }
    else
    {
      words.push_back(w);
    }
  }

  if (tup.connectedNameIndexOpt)
  {
    const auto &connectedName = anka::getValue<std::string>(context, tup.connectedNameIndexOpt.value());
    auto &&internal_functions = anka::getInternalFunctions();

    auto kv = ranges::views::keys(internal_functions);
    std::vector<anka::InternalFunctionDefinition> definitions{kv.begin(), kv.end()};

    auto found = definitions |
                 ranges::views::filter([&connectedName](const auto &def) { return def.name == connectedName; }) |
                 ranges::to<std::vector<anka::InternalFunctionDefinition>>;

    if (!found.empty())
    {
      auto maxArgIter = std::max_element(found.begin(), found.end(), [](const auto &def1, const auto &def2) {
        return def1.argumentTypes.size() < def2.argumentTypes.size();
      });

      int nrWordsToInsert =
          std::min(static_cast<int>(maxArgIter->argumentTypes.size()) - static_cast<int>(tup.words.size()),
                   static_cast<int>(wordsToBeInserted.size()));
      if (nrWordsToInsert > 0)
      {
        words.insert(words.end(), wordsToBeInserted.begin(), wordsToBeInserted.begin() + nrWordsToInsert);
      }
    }
  }

  return anka::createWord(context, {std::move(words), false});
}

auto createExecutorBlocks(anka::Context &context, const std::vector<anka::Word> &executorWords, const anka::Word &rhs)
    -> std::vector<std::vector<anka::Word>>
{
  using namespace anka;

  std::vector<std::vector<anka::Word>> sentences;
  for (auto &&lhs : executorWords)
  {
    if (lhs.type == WordType::Tuple)
    {
      auto &&tup = getValue<const Tuple &>(context, lhs.index);
      if (tup.connectedNameIndexOpt)
      {
        sentences.back().push_back(lhs);
      }
      else
      {
        sentences.push_back({lhs});
      }
    }
    else
    {
      sentences.push_back({lhs});
    }
  }
  for (auto &&sentence : sentences)
  {
    sentence.push_back(rhs);
  }
  return sentences;
}

auto foldExecutor(anka::Context &context, const anka::Word &w1, const anka::Word &w2) -> anka::Word
{
  using namespace anka;
  auto &&blocks = createExecutorBlocks(context, anka::getValue<const anka::Executor &>(context, w2.index).words, w1);

  std::vector<Word> res;
  for (auto &&block : blocks)
  {
    res.push_back(executeWords(context, block).value());
  }
  return createWord(context, Tuple{res, false});
}

auto foldFunction(anka::Context &context, const ExecutionInformation &info) -> std::optional<anka::Word>
{
  return info.executer(context, info.allWords, info.interpretation.expandArray);
}

auto checkIfNameIsAvailable(anka::Context &context, const std::string &name) -> bool
{
  if (!anka::getInternalFunctionDefinitionsWithName(name).empty())
    return false;
  if (context.userDefinedNames.contains(name))
    return false;
  return true;
}

auto getUnwrapedFoldableWord(const anka::Context &context, const anka::Word &word) -> anka::Word
{
  auto opt = anka::getFoldableWord(context, word);
  if (opt)
  {
    return opt.value();
  }

  throw anka::ExecutionError(word, std::nullopt, "Could not fold words");
}

auto fold(anka::Context &context, const anka::Word &lhs, const anka::Word &rhs) -> anka::Word
{
  using namespace anka;
  if (context.assignNext)
  {
    context.assignNext = false;

    if (lhs.type != WordType::Name)
      throw anka::ExecutionError{lhs, rhs, "Could not assign to non-name word."};

    const auto &name = context.names[lhs.index];
    if (!checkIfNameIsAvailable(context, name))
      throw anka::ExecutionError{lhs, std::nullopt, "Name already taken."};

    context.userDefinedNames[name] = getUnwrapedFoldableWord(context, rhs);
    return lhs;
  }

  if (lhs.type == WordType::Assignment)
  {
    context.assignNext = true;
    return rhs;
  }

  if (rhs.type == WordType::Name)
  {
    return fold(context, lhs, getUnwrapedFoldableWord(context, rhs));
  }

  if (lhs.type == WordType::Name)
  {
    const auto &name = context.names[lhs.index];
    if (context.userDefinedNames.contains(name))
      return fold(context, getUnwrapedFoldableWord(context, lhs), rhs);
  }

  if (rhs.type == WordType::Block)
  {
    auto &&block = getValue<const Block &>(context, rhs.index);
    auto words = std::vector<Word>{lhs};
    words.insert(words.end(), block.words.begin(), block.words.end());
    return executeWords(context, words).value();
  }

  if (lhs.type == WordType::Name)
  {
    const auto &name = context.names[lhs.index];
    auto interpretation = findOverload(context, name, rhs);
    if (interpretation)
    {
      auto wordOpt = foldFunction(context, interpretation.value());
      if (!wordOpt)
      {
        throw anka::ExecutionError{rhs, lhs, "Could not fold words."};
      }
      return wordOpt.value();
    }
    throw anka::ExecutionError{std::nullopt, lhs, "Could not find name."};
  }
  else if (lhs.type == WordType::Tuple)
  {
    return foldtuple(context, rhs, lhs);
  }
  else if (lhs.type == WordType::PlaceHolder)
  {
    return foldPlaceholder(context, lhs, rhs);
  }
  else if (lhs.type == WordType::Executor)
  {
    return foldExecutor(context, rhs, lhs);
  }
  else if (lhs.type == WordType::Block)
  {
    auto &&block = getValue<const Block &>(context, lhs.index);
    auto words = block.words;
    words.push_back(rhs);
    return executeWords(context, words).value();
  }

  throw anka::ExecutionError{rhs, lhs, "Could not fold words."};
}

auto executeWords(anka::Context &context, const std::vector<anka::Word> &words) -> std::optional<anka::Word>
{
  using namespace ranges;
  using namespace anka;

  if (words.empty())
    return std::nullopt;

  const auto &init = words.back();
  auto rv = words | views::reverse | views::drop(1);

  return std::accumulate(rv.begin(), rv.end(), init,
                         [&context](const Word &rhs, const Word &lhs) { return fold(context, lhs, rhs); });
}

auto anka::execute(Context &context, const std::vector<Sentence> &sentences) -> std::optional<Word>
{
  if (sentences.empty())
    return std::nullopt;

  std::optional<Word> wordOpt = std::nullopt;
  for (const auto &sentence : sentences)
  {
    if (sentence.words.empty())
      continue;

    wordOpt = executeWords(context, sentence.words);
    if (wordOpt.has_value())
      wordOpt = getFoldableWord(context, wordOpt.value());
  }

  return wordOpt;
}
