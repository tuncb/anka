#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <range/v3/view/transform.hpp>

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
  Tuple,
  PlaceHolder,
  Executor,
  Block,
  Assignment
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

struct Block
{
  std::vector<Word> words;
};

struct Tuple
{
  std::vector<Word> words;
  std::optional<size_t> connectedNameIndexOpt;
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
  std::unordered_map<std::string, Word> userDefinedNames;
  std::vector<std::string> names;
  std::vector<Tuple> tuples;
  std::vector<Executor> executors;
  std::vector<Block> blocks;

  bool assignNext = false;
};

template <typename R, typename T> using BinaryOpt = R (*)(T, T);
template <typename T> using FilterFunc = bool (*)(T);

template <typename T>
struct ValueReturnType
{
  using ReturnType = T;
};

template <> struct ValueReturnType<std::vector<bool>>
{
  using ReturnType = const std::vector<bool>&;
};

template <> struct ValueReturnType<std::vector<int>>
{
  using ReturnType = const std::vector<int> &;
};

template <> struct ValueReturnType<std::vector<double>>
{
  using ReturnType = const std::vector<double> &;
};

template <> struct ValueReturnType<bool>
{
  using ReturnType = bool;
};

template <> struct ValueReturnType<int>
{
  using ReturnType = int;
};

template <> struct ValueReturnType<double>
{
  using ReturnType = double;
};

template <typename T> auto getValue(const Context &context, size_t index) -> ValueReturnType<T>::ReturnType
{
  using Decayed = std::remove_cv<typename std::remove_reference<T>::type>::type;

  if constexpr (std::is_same_v<Decayed, int>)
    return context.integerNumbers[index];
  else if constexpr (std::is_same_v<Decayed, std::string>)
    return context.names[index];
  else if constexpr (std::is_same_v<Decayed, std::vector<int>>)
    return context.integerArrays[index];
  else if constexpr (std::is_same_v<Decayed, Tuple>)
    return context.tuples[index];
  else if constexpr (std::is_same_v<Decayed, Executor>)
    return context.executors[index];
  else if constexpr (std::is_same_v<Decayed, Block>)
    return context.blocks[index];
  else if constexpr (std::is_same_v<Decayed, bool>)
    return context.booleans[index];
  else if constexpr (std::is_same_v<Decayed, std::vector<bool>>)
    return context.booleanArrays[index];
  else if constexpr (std::is_same_v<Decayed, double>)
    return context.doubleNumbers[index];
  else if constexpr (std::is_same_v<Decayed, std::vector<double>>)
    return context.doubleArrays[index];
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getValue().");
    }
  ();
}

template <typename T> auto getItemSize(const Context &context, size_t index) -> size_t
{
  using Decayed = std::remove_cv<typename std::remove_reference<T>::type>::type;

  if constexpr (std::is_same_v<Decayed, std::vector<int>>)
    return context.integerArrays[index].size();
  else if constexpr (std::is_same_v<Decayed, std::vector<bool>>)
    return context.booleanArrays[index].size();
  else if constexpr (std::is_same_v<Decayed, std::vector<double>>)
    return context.doubleArrays[index].size();
  else
    return 1;
}

template <typename T> constexpr auto isExpandable() -> bool
{
  using Decayed = std::remove_cv<typename std::remove_reference<T>::type>::type;

  if constexpr (std::is_same_v<Decayed, int>)
    return true;
  else if constexpr (std::is_same_v<Decayed, bool>)
    return true;
  else if constexpr (std::is_same_v<Decayed, double>)
    return true;
  else
    return false;
}

auto parseAST(const std::string_view content, std::span<Token> tokens, Context &context) -> std::vector<Sentence>;

auto toString(const anka::Context &context, const anka::Word &word) -> std::string;
auto toString(anka::WordType type) -> std::string;

auto createWord(Context &context, int value) -> Word;
auto createWord(Context &context, bool value) -> Word;
auto createWord(Context &context, double value) -> Word;
auto createWord(Context &context, std::vector<int> &&vec) -> Word;
auto createWord(Context &context, std::vector<bool> &&vec) -> Word;
auto createWord(Context &context, std::vector<double> &&vec) -> Word;
auto createWord(Context &context, Tuple &&tuple) -> Word;
auto createWord(Context &context, Executor &&executor) -> Word;
auto createWord(Context &context, Block &&block) -> Word;

auto getFoldableWord(const anka::Context &context, const anka::Word &word) -> std::optional<anka::Word>;
auto getWord(const anka::Context &context, const anka::Word &input, size_t index) -> std::optional<anka::Word>;
auto getAllWords(const anka::Context &context, const anka::Word &input) -> std::vector<anka::Word>;
auto getWordCount(const anka::Context &context, const anka::Word &word) -> size_t;
auto getWordTypes(const std::vector<anka::Word> &words) -> std::vector<anka::WordType>;

auto injectInternalConstants(Context &context) -> void;

template <typename T> anka::WordType getWordType()
{
  using Decayed = std::remove_cv<typename std::remove_reference<T>::type>::type;

  using namespace anka;
  if constexpr (std::is_same_v<Decayed, int>)
    return WordType::IntegerNumber;
  else if constexpr (std::is_same_v<Decayed, double>)
    return WordType::DoubleNumber;
  else if constexpr (std::is_same_v<Decayed, std::string>)
    return WordType::Name;
  else if constexpr (std::is_same_v<Decayed, std::vector<int>>)
    return WordType::IntegerArray;
  else if constexpr (std::is_same_v<Decayed, std::vector<double>>)
    return WordType::DoubleArray;
  else if constexpr (std::is_same_v<Decayed, bool>)
    return WordType::Boolean;
  else if constexpr (std::is_same_v<Decayed, std::vector<bool>>)
    return WordType::BooleanArray;
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getWordType().");
    }
  ();
}

template <typename T> tl::optional<T> extractValue(const anka::Context &context, const anka::Word &input, size_t index)
{
  using namespace anka;
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

} // namespace anka