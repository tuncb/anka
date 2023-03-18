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
  Context
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
  std::vector<Sentence> sentences;

  std::vector<int> integerNumbers;
  std::vector<std::vector<int>> integerArrays;
};

auto createAST(std::string_view content, std::span<Token> tokens) -> Context;

auto toString(const anka::Context &context, const anka::Word &word) -> std::string;

} // namespace anka