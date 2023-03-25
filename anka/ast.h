#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <tl/optional.hpp>

#include "tokenizer.h"

namespace anka
{

struct ASTError
{
  std::optional<Token> tokenOpt;
  std::string message;
};

enum class WordType
{
  IntegerNumber,
  IntegerArray,
  DoubleNumber,
  DoubleArray,
  Boolean,
  BooleanArray,
  Name,
  Context,
  Tuple,
  PlaceHolder,
  Executor
};

struct Word
{
  WordType type;
  size_t index;

  auto operator<=>(const Word &) const = default;
};

struct Sentence
{
  std::vector<Word> words;
};

struct Tuple
{
  std::vector<Word> words;
  bool isConnected;
};

struct Executor
{
  std::vector<Word> words;
};

struct PlaceHolder
{
  size_t index;
};

struct Context
{
  std::vector<int> integerNumbers;
  std::vector<std::vector<int>> integerArrays;
  std::vector<double> doubleNumbers;
  std::vector<std::vector<double>> doubleArrays;
  std::vector<bool> booleans;
  std::vector<std::vector<bool>> booleanArrays;
  std::vector<std::string> names;
  std::vector<Tuple> tuples;
  std::vector<Executor> executors;
};

template <typename T> T getValue(const Context &context, size_t index)
{
  if constexpr (std::is_same_v<T, int>)
    return context.integerNumbers[index];
  else if constexpr (std::is_same_v<T, std::string>)
    return context.names[index];
  else if constexpr (std::is_same_v<T, const std::vector<int> &>)
    return context.integerArrays[index];
  else if constexpr (std::is_same_v<T, const Tuple &>)
    return context.tuples[index];
  else if constexpr (std::is_same_v<T, const Executor &>)
    return context.executors[index];
  else if constexpr (std::is_same_v<T, bool>)
    return context.booleans[index];
  else if constexpr (std::is_same_v<T, const std::vector<bool> &>)
    return context.booleanArrays[index];
  else if constexpr (std::is_same_v<T, double>)
    return context.doubleNumbers[index];
  else if constexpr (std::is_same_v<T, const std::vector<double> &>)
    return context.doubleArrays[index];
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getValue().");
    }
  ();
}

struct AST
{
  Context context;
  std::vector<Sentence> sentences;
};

auto parseAST(const std::string_view content, std::span<Token> tokens, Context &&context) -> AST;

auto toString(const anka::Context &context, const anka::Word &word) -> std::string;

auto createWord(Context &context, int value) -> Word;
auto createWord(Context &context, bool value) -> Word;
auto createWord(Context &context, double value) -> Word;
auto createWord(Context &context, std::vector<int> &&vec) -> Word;
auto createWord(Context &context, std::vector<bool> &&vec) -> Word;
auto createWord(Context &context, std::vector<double> &&vec) -> Word;
auto createWord(Context &context, Tuple &&tuple) -> Word;
auto createWord(Context &context, Executor &&executor) -> Word;

auto getWord(const Context &context, const Word &input, size_t index) -> std::optional<Word>;
auto getAllWords(const Context &context, const Word &input) -> std::vector<Word>;

template <typename T> WordType getWordType()
{
  if constexpr (std::is_same_v<T, int>)
    return WordType::IntegerNumber;
  else if constexpr (std::is_same_v<T, double>)
    return WordType::DoubleNumber;
  else if constexpr (std::is_same_v<T, std::string>)
    return WordType::Name;
  else if constexpr (std::is_same_v<T, const std::vector<int> &>)
    return WordType::IntegerArray;
  else if constexpr (std::is_same_v<T, const std::vector<double> &>)
    return WordType::DoubleArray;
  else if constexpr (std::is_same_v<T, bool>)
    return WordType::Boolean;
  else if constexpr (std::is_same_v<T, const std::vector<bool> &>)
    return WordType::BooleanArray;
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getWordType().");
    }
  ();
}

template <typename T> tl::optional<T> extractValue(const Context &context, const Word &input, size_t index)
{
  if (index > 0 && input.type != WordType::Tuple)
    return tl::nullopt;

  const auto wOpt = getWord(context, input, index);
  if (!wOpt)
    return tl::nullopt;

  auto &&w = wOpt.value();
  if (w.type != getWordType<T>())
    return tl::nullopt;

  return getValue<T>(context, w.index);
}

auto getWordCount(const Context &context, const Word &word) -> size_t;

} // namespace anka