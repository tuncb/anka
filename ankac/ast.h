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
  Name,
  Context,
  Tuple
};

enum class InternalFunctionType
{
  IntToIntArray,
  IntToInt,
  IntArrayToInt,
  IntArrayToIntArray,
};

struct InternalFunction
{
  void *ptr = nullptr;
  InternalFunctionType type;
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

struct Context
{
  std::vector<int> integerNumbers;
  std::vector<std::vector<int>> integerArrays;
  std::vector<std::string> names;
  std::vector<std::vector<Word>> tuples;
};

template <typename T> T getValue(const Context &context, size_t index)
{
  if constexpr (std::is_same_v<T, int>)
    return context.integerNumbers[index];
  else if constexpr (std::is_same_v<T, std::string>)
    return context.names[index];
  else if constexpr (std::is_same_v<T, const std::vector<int> &>)
    return context.integerArrays[index];
  else if constexpr (std::is_same_v<T, const std::vector<anka::Word> &>)
    return context.tuples[index];
  else
    []<bool flag = false>()
    {
      static_assert(flag, "no match");
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
auto toString(TokenType type) -> std::string;

auto createWord(Context &context, int value) -> Word;
auto createWord(Context &context, std::vector<int> &&vec) -> Word;
auto createWord(Context &context, std::vector<anka::Word> &&vec) -> Word;

auto getWord(const Context &context, const Word &input, size_t index) -> std::optional<Word>;

template <typename T> WordType getWordType()
{
  if constexpr (std::is_same_v<T, int>)
    return WordType::IntegerNumber;
  else if constexpr (std::is_same_v<T, std::string>)
    return WordType::Name;
  else if constexpr (std::is_same_v<T, const std::vector<int> &>)
    return WordType::IntegerArray;
  else if constexpr (std::is_same_v<T, const std::vector<anka::Word> &>)
    return WordType::Tuple;
  else
    []<bool flag = false>()
    {
      static_assert(flag, "no match");
    }
  ();
}

template <typename T> tl::optional<T> extractValue(const Context &context, const Word &input, size_t index)
{
  const auto wOpt = getWord(context, input, index);
  if (!wOpt)
    return tl::nullopt;

  auto w = wOpt.value();
  if (w.type != getWordType<T>())
    return tl::nullopt;

  return getValue<T>(context, w.index);
}

} // namespace anka