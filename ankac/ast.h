#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>

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
  Context
};

enum class InternalFunctionType
{
  IntToIntArray,
  IntToInt,
  IntArrayToInt,
};

struct InternalFunction
{
  void* ptr = nullptr;
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
};

struct AST
{
  Context context;
  std::vector<Sentence> sentences;
};

auto parseAST(const std::string_view content, std::span<Token> tokens, Context &&context) -> AST;

auto toString(const anka::Context &context, const anka::Word &word) -> std::string;

} // namespace anka