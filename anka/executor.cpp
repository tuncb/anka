#include "executor.h"

#include <format>
#include <numeric>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>

import anka;

// forward declerations
auto executeWords(anka::Context &context, const std::vector<anka::Word> &words) -> std::optional<anka::Word>;

auto nameExists(const std::string &name) -> bool
{
  const auto &internalFunctions = anka::getInternalFunctions();
  auto iter = internalFunctions.find(name);
  return iter != internalFunctions.end();
}

auto findOverload(const anka::Context &context, const std::string &name, const anka::Word &word)
    -> std::optional<anka::InternalFunction>
{
  const auto &internalFunctions = anka::getInternalFunctions();
  if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
  {
    for (auto &&overload : iter->second)
    {
      if (checkOverloadCompatibility(context, overload.type, word))
        return overload;
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
    if (auto iter = internal_functions.find(connectedName); iter != internal_functions.end())
    {
      auto maxArgumentFuncIter =
          std::max_element(iter->second.begin(), iter->second.end(),
                           [](const anka::InternalFunction &f1, const anka::InternalFunction &f2) {
                             return anka::nrArguments(f1.type) < anka::nrArguments(f2.type);
                           });
      int nrWordsToInsert = std::min(anka::nrArguments(maxArgumentFuncIter->type) - static_cast<int>(tup.words.size()),
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

auto checkIfNameIsAvailable(anka::Context &context, const std::string &name) -> bool
{
  if (anka::getInternalFunctions().contains(name))
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
    auto functOpt = findOverload(context, name, rhs);
    if (functOpt)
    {
      auto wordOpt = foldFunction(context, rhs, functOpt.value());
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
