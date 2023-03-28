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
  Tuple,
  PlaceHolder,
  Executor,
  Block,
  Assignment,
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
  std::unordered_map<std::string, Word> userDefinedNames;
  std::vector<std::string> names;
  std::vector<Tuple> tuples;
  std::vector<Executor> executors;
  std::vector<Block> blocks;

  bool assignNext = false;
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
  else if constexpr (std::is_same_v<T, const Block &>)
    return context.blocks[index];
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

auto parseAST(const std::string_view content, std::span<Token> tokens, Context &context) -> std::vector<Sentence>;

auto toString(const anka::Context &context, const anka::Word &word) -> std::string;

auto createWord(Context &context, int value) -> Word;
auto createWord(Context &context, bool value) -> Word;
auto createWord(Context &context, double value) -> Word;
auto createWord(Context &context, std::vector<int> &&vec) -> Word;
auto createWord(Context &context, std::vector<bool> &&vec) -> Word;
auto createWord(Context &context, std::vector<double> &&vec) -> Word;
auto createWord(Context &context, Tuple &&tuple) -> Word;
auto createWord(Context &context, Executor &&executor) -> Word;
auto createWord(Context &context, Block &&block) -> Word;

auto injectInternalConstants(Context &context) -> void;

} // namespace anka