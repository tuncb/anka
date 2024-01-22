module;
#include <algorithm>
#include <cassert>
#include <charconv>
#include <format>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <fmt/ranges.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/transform.hpp>

#include <tl/optional.hpp>

export module anka:interpreter_state;

import :tokenizer;

namespace anka
{
export struct ASTError
{
  std::optional<Token> tokenOpt;
  std::string message;
};

export enum class WordType
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

export struct Word
{
  WordType type;
  size_t index;

  auto operator<=>(const Word &) const = default;
};

export struct Sentence
{
  std::vector<Word> words;
};

export struct Block
{
  std::vector<Word> words;
};

export struct Tuple
{
  std::vector<Word> words;
  std::optional<size_t> connectedNameIndexOpt;
};

export struct Executor
{
  std::vector<Word> words;
};

export struct PlaceHolder
{
  size_t index;
};

export struct Context
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

export template <typename T> struct ValueReturnType
{
  using ReturnType = T;
};

export template <> struct ValueReturnType<std::vector<bool>>
{
  using ReturnType = const std::vector<bool> &;
};

export template <> struct ValueReturnType<std::vector<int>>
{
  using ReturnType = const std::vector<int> &;
};

export template <> struct ValueReturnType<std::vector<double>>
{
  using ReturnType = const std::vector<double> &;
};

export template <> struct ValueReturnType<bool>
{
  using ReturnType = bool;
};

export template <> struct ValueReturnType<int>
{
  using ReturnType = int;
};

export template <> struct ValueReturnType<double>
{
  using ReturnType = double;
};

export template <typename T> auto getValue(const Context &context, size_t index) -> ValueReturnType<T>::ReturnType
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

export template <typename T> auto getItemSize(const Context &context, size_t index) -> size_t
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

export template <typename T> constexpr auto isExpandable() -> bool
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

template <class T>
concept TokenForwardIterator =
    std::forward_iterator<T> && std::is_same_v<typename std::iterator_traits<T>::value_type, anka::Token>;

// Forward declerations
auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd, std::optional<size_t> connectedNameOpt) -> anka::Word;
auto extractArray(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;
auto extractExecutor(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                     TokenForwardIterator auto tokensEnd) -> anka::Word;
auto extractBlock(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;

template <typename T> auto throwNumberTokenError(const std::string_view content, anka::Token token) -> T
{
  auto b = &(content[token.start]);
  auto e = b + token.len;
  auto msg = std::format("Expected number, found: {}", std::string(b, e));
  throw anka::ASTError({token, msg});
}

template <typename T> auto toNumber(const std::string_view content, size_t pos, size_t len) -> std::optional<T>
{
  auto b = &(content[pos]);
  auto e = b + len;
  T value = 0;
  auto res = std::from_chars(b, e, value);
  if (res.ec == std::errc{} && res.ptr == e)
  {
    return value;
  }

  return std::nullopt;
}

template <typename T> auto toNumber(const std::string_view content, anka::Token token) -> T
{
  auto valueOpt = toNumber<T>(content, token.start, token.len);
  if (valueOpt)
    return valueOpt.value();

  return throwNumberTokenError<T>(content, token);
}

export auto createWord(Context &context, int value) -> Word
{
  context.integerNumbers.push_back(value);
  return anka::Word{anka::WordType::IntegerNumber, context.integerNumbers.size() - 1};
}

export auto createWord(Context &context, bool value) -> Word
{
  context.booleans.push_back(value);
  return anka::Word{anka::WordType::Boolean, context.booleans.size() - 1};
}

export auto createWord(Context &context, double value) -> Word
{
  context.doubleNumbers.push_back(value);
  return anka::Word{anka::WordType::DoubleNumber, context.doubleNumbers.size() - 1};
}

export auto createWord(Context &context, Tuple &&tuple) -> Word
{
  context.tuples.push_back(std::move(tuple));
  return anka::Word{anka::WordType::Tuple, context.tuples.size() - 1};
}

export auto createWord(Context &context, Executor &&executor) -> Word
{
  context.executors.push_back(std::move(executor));
  return anka::Word{anka::WordType::Executor, context.executors.size() - 1};
}

export auto createWord(Context &context, Block &&block) -> Word
{
  context.blocks.push_back(std::move(block));
  return anka::Word{anka::WordType::Block, context.blocks.size() - 1};
}

export auto createWord(Context &context, std::vector<int> &&vec) -> Word
{
  context.integerArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::IntegerArray, context.integerArrays.size() - 1};
}

export auto createWord(Context &context, std::vector<bool> &&vec) -> Word
{
  context.booleanArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::BooleanArray, context.booleanArrays.size() - 1};
}

export auto createWord(Context &context, std::vector<double> &&vec) -> Word
{
  context.doubleArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::DoubleArray, context.doubleArrays.size() - 1};
}

template <typename T>
auto addNumberWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
{
  auto token = *(tokenIter++);
  auto value = toNumber<T>(content, token);
  return createWord(context, value);
}

auto addPlaceHolderWord(const std::string_view content, TokenForwardIterator auto &tokenIter) -> anka::Word
{
  auto token = *(tokenIter++);
  auto indexOpt = token.len == 1 ? 0 : toNumber<size_t>(content, token.start + 1, token.len - 1);

  if (indexOpt)
    return anka::Word{anka::WordType::PlaceHolder, indexOpt.value()};

  return throwNumberTokenError<anka::Word>(content, token);
}

auto addNameWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
{
  auto token = *(tokenIter++);
  auto value = std::string(content.data() + token.start, content.data() + token.start + token.len);

  if (value == "true")
  {
    return createWord(context, true);
  }

  if (value == "false")
  {
    return createWord(context, false);
  }

  context.names.push_back(value);
  return {anka::WordType::Name, context.names.size() - 1};
}

auto extractWords(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd, const std::vector<anka::TokenType> &unexpectedTypes,
                  anka::TokenType finisherType) -> std::vector<anka::Word>
{
  using namespace anka;

  std::vector<anka::Word> words;
  std::optional<size_t> connectedNameIndexOpt;
  for (; tokenIter != tokensEnd;)
  {
    if (tokenIter->type == finisherType)
    {
      ++tokenIter;
      return words;
    }
    if (std::ranges::find(unexpectedTypes, tokenIter->type) != unexpectedTypes.end())
    {
      throw anka::ASTError{*tokenIter,
                           std::format("Did not expect to find token: {}", anka::toString(tokenIter->type))};
    }

    switch (tokenIter->type)
    {
    case TokenType::NumberInt:
      words.emplace_back(addNumberWord<int>(content, context, tokenIter));
      break;
    case TokenType::NumberDouble:
      words.emplace_back(addNumberWord<double>(content, context, tokenIter));
      break;
    case TokenType::Placeholder:
      words.emplace_back(addPlaceHolderWord(content, tokenIter));
      break;
    case TokenType::Name:
      words.emplace_back(addNameWord(content, context, tokenIter));
      break;
    case TokenType::Connector:
      assert(!words.empty());
      assert(words.back().type == anka::WordType::Name);
      connectedNameIndexOpt = words.back().index;
      tokenIter++;
      break;
    case TokenType::Assignment:
      words.emplace_back(Word{WordType::Assignment, 0});
      ++tokenIter;
      break;
    case TokenType::ArrayStart:
      ++tokenIter;
      words.emplace_back(extractArray(content, context, tokenIter, tokensEnd));
      break;
    case TokenType::BlockStart:
      ++tokenIter;
      words.emplace_back(extractBlock(content, context, tokenIter, tokensEnd));
      break;
    case TokenType::Executor:
      ++tokenIter;
      words.emplace_back(extractExecutor(content, context, tokenIter, tokensEnd));
      break;
    case TokenType::TupleStart:
      ++tokenIter;
      words.emplace_back(extractTuple(content, context, tokenIter, tokensEnd, connectedNameIndexOpt));
      connectedNameIndexOpt = std::nullopt;
      break;
    default:
      throw anka::ASTError{*tokenIter,
                           std::format("Did not expect to find token: {}", anka::toString(tokenIter->type))};
    }
  }

  return words;
}

auto extractSentence(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                     TokenForwardIterator auto tokensEnd) -> anka::Sentence
{
  using namespace anka;

  Sentence sentence{extractWords(content, context, tokenIter, tokensEnd, {TokenType::TupleEnd, TokenType::ArrayEnd},
                                 TokenType::SentenceEnd)};
  return sentence;
}

auto extractBlock(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word
{
  using namespace anka;
  auto unexpectedTypes = std::vector<TokenType>{
      TokenType::SentenceEnd,
  };

  return createWord(context,
                    Block{extractWords(content, context, tokenIter, tokensEnd, unexpectedTypes, TokenType::BlockEnd)});
}

auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd, std::optional<size_t> connectedNameOpt) -> anka::Word
{
  using namespace anka;

  return createWord(context,
                    anka::Tuple{extractWords(content, context, tokenIter, tokensEnd,
                                             {TokenType::SentenceEnd, TokenType::ArrayEnd}, TokenType::TupleEnd),
                                connectedNameOpt});
}

auto extractExecutor(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                     TokenForwardIterator auto tokensEnd) -> anka::Word
{
  using namespace anka;

  auto unexpectedTypes = std::vector<TokenType>{TokenType::ArrayStart,   TokenType::ArrayEnd,    TokenType::NumberInt,
                                                TokenType::NumberDouble, TokenType::SentenceEnd, TokenType::TupleEnd};
  return createWord(context, anka::Executor{extractWords(content, context, tokenIter, tokensEnd, unexpectedTypes,
                                                         TokenType::Executor)});
}

auto extractArray(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word
{
  using namespace anka;

  auto startToken = *tokenIter;

  Context arrayContext;
  auto words = extractWords(content, arrayContext, tokenIter, tokensEnd, {TokenType::SentenceEnd, TokenType::TupleEnd},
                            TokenType::ArrayEnd);

  if (words.empty())
  {
    throw ASTError{startToken, "Empty arrays are not supported"};
  }

  auto expectedType = words.front().type;
  if (!(expectedType == WordType::IntegerNumber || expectedType == WordType::Boolean ||
        expectedType == WordType::DoubleNumber))
  {
    throw ASTError{startToken, "Only boolean, integer or double arrays are supported"};
  }

  if (!std::all_of(words.begin(), words.end(), [expectedType](const Word &word) { return word.type == expectedType; }))
  {
    throw ASTError{startToken, "Arrays should have elements of the same type"};
  }

  if (expectedType == WordType::IntegerNumber)
  {
    return createWord(context, std::move(arrayContext.integerNumbers));
  }

  if (expectedType == WordType::Boolean)
  {
    return createWord(context, std::move(arrayContext.booleans));
  }

  if (expectedType == WordType::DoubleNumber)
  {
    return createWord(context, std::move(arrayContext.doubleNumbers));
  }

  throw ASTError{startToken, "Fatal Error: Could not extract array"};
};

export auto parseAST(const std::string_view content, std::span<Token> tokens, Context &context)
    -> std::vector<Sentence>
{
  std::vector<Sentence> sentences;
  for (auto tokenIter = tokens.begin(); tokenIter != tokens.end();)
  {
    sentences.emplace_back(extractSentence(content, context, tokenIter, tokens.end()));
  }

  return sentences;
}

export auto toString(const anka::Context &context, const anka::Word &word) -> std::string;
export auto toString(anka::WordType type) -> std::string;

export auto getFoldableWord(const anka::Context &context, const anka::Word &word) -> std::optional<anka::Word>;
export auto getWord(const anka::Context &context, const anka::Word &input, size_t index) -> std::optional<anka::Word>;
export auto getAllWords(const anka::Context &context, const anka::Word &input) -> std::vector<anka::Word>;
export auto getWordCount(const anka::Context &context, const anka::Word &word) -> size_t;
export auto getWordTypes(const std::vector<anka::Word> &words) -> std::vector<anka::WordType>;

export template <typename T> anka::WordType getWordType()
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

export template <typename T>
tl::optional<T> extractValue(const anka::Context &context, const anka::Word &input, size_t index)
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

auto anka::toString(anka::WordType type) -> std::string
{
  switch (type)
  {
  case anka::WordType::Assignment:
    return "=";
  case anka::WordType::IntegerNumber:
    return "int";
  case anka::WordType::IntegerArray:
    return "(int)";
  case anka::WordType::DoubleNumber:
    return "double";
  case anka::WordType::DoubleArray:
    return "(double)";
  case anka::WordType::Boolean:
    return "bool";
  case anka::WordType::BooleanArray:
    return "(bool)";
  case anka::WordType::Name:
    return "name";
  case anka::WordType::Tuple:
    return "tuple";
  case anka::WordType::PlaceHolder:
    return "_X";
  case anka::WordType::Executor:
    return "||";
  case anka::WordType::Block:
  default:
    return "unknownType";
  }
}





auto anka::getWord(const anka::Context &context, const anka::Word &input, size_t index) -> std::optional<anka::Word>
{
  using namespace anka;
  auto candidate = input;
  if (input.type == WordType::Tuple)
  {
    const auto &tup = context.tuples[input.index];
    if (tup.words.size() <= index)
      return std::nullopt;
    candidate = tup.words[index];
  }

  if (candidate.type == WordType::Name)
  {
    return getFoldableWord(context, candidate);
  }

  return candidate;
}

auto anka::getAllWords(const anka::Context &context, const anka::Word &input) -> std::vector<anka::Word>
{
  using namespace anka;
  if (input.type == WordType::Tuple)
  {
    const auto &tup = context.tuples[input.index];
    return tup.words;
  }

  return {input};
}

auto anka::getWordCount(const anka::Context &context, const anka::Word &word) -> size_t
{
  using namespace anka;
  if (word.type != WordType::Tuple)
  {
    return 1;
  }

  return getValue<const Tuple &>(context, word.index).words.size();
}
auto formatDouble(double value) -> std::string
{
  auto str = std::format("{:.5f}", value);
  const auto pos = str.find_last_not_of('0');
  if (pos >= str.length() - 1)
    return str;

  str.erase(str.find_last_not_of('0') + 2, std::string::npos);

  auto dotPos = str.find_first_of('.');

  if (str.length() - dotPos > 2 && str.back() == '0')
    str.pop_back();

  return str;
}


auto anka::getWordTypes(const std::vector<anka::Word> &words) -> std::vector<anka::WordType>
{
  return words | ranges::views::transform([](const auto &word) { return word.type; }) |
         ranges::to<std::vector<anka::WordType>>;
}
} // namespace anka