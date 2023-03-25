#include "ast.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <iterator>

#include <fmt/ranges.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>

#include "internal_functions.h"

template <class T>
concept TokenForwardIterator =
    std::forward_iterator<T> && std::is_same_v<typename std::iterator_traits<T>::value_type, anka::Token>;

// Forward declerations
auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd, bool isConnected) -> anka::Word;
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

template <typename T>
auto addNumberWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
{
  auto token = *(tokenIter++);
  auto value = toNumber<T>(content, token);
  return createWord(context, value);
}

auto addPlaceHolderWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
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
  auto isConnected = false;
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
      words.emplace_back(addPlaceHolderWord(content, context, tokenIter));
      break;
    case TokenType::Name:
      words.emplace_back(addNameWord(content, context, tokenIter));
      break;
    case TokenType::Connector:
      isConnected = true;
      tokenIter++;
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
      words.emplace_back(extractTuple(content, context, tokenIter, tokensEnd, isConnected));
      isConnected = false;
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
                  TokenForwardIterator auto tokensEnd, bool isConnected) -> anka::Word
{
  using namespace anka;

  return createWord(context,
                    anka::Tuple{extractWords(content, context, tokenIter, tokensEnd,
                                             {TokenType::SentenceEnd, TokenType::ArrayEnd}, TokenType::TupleEnd),
                                isConnected});
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

auto anka::parseAST(const std::string_view content, std::span<Token> tokens, Context &&context) -> AST
{
  std::vector<Sentence> sentences;
  for (auto tokenIter = tokens.begin(); tokenIter != tokens.end();)
  {
    sentences.emplace_back(extractSentence(content, context, tokenIter, tokens.end()));
  }

  return {std::move(context), sentences};
}

auto formatDouble(double value) -> std::string
{
  auto str = std::format("{:.5f}", value);
  const auto pos = str.find_last_not_of('0');
  if (pos >= str.length() - 1)
    return str;

  str.erase(str.find_last_not_of('0') + 2, std::string::npos);

  if (str.length() > 3 && str.back() == '0')
    str.pop_back();

  return str;
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
    auto &&internalFunctions = getInternalFunctions();
    if (auto iter = internalFunctions.find(name); iter != internalFunctions.end())
    {
      if (iter->second.size() == 1)
      {
        return fmt::format("{}: {}", name, toString(iter->second.front().type));
      }
      else
      {
        auto toStr = [&name](const anka::InternalFunction &overload) {
          return fmt::format("{}: {}", name, toString(overload.type));
        };

        auto definitions = iter->second | ranges::views::transform(toStr) | ranges::to<std::vector<std::string>>();
        return fmt::format("{}", fmt::join(definitions, "\n"));
      }
    }
    return name;
  }
  default:
    return "";
  }
}

auto anka::createWord(Context &context, int value) -> Word
{
  context.integerNumbers.push_back(value);
  return anka::Word{anka::WordType::IntegerNumber, context.integerNumbers.size() - 1};
}

auto anka::createWord(Context &context, bool value) -> Word
{
  context.booleans.push_back(value);
  return anka::Word{anka::WordType::Boolean, context.booleans.size() - 1};
}

auto anka::createWord(Context &context, double value) -> Word
{
  context.doubleNumbers.push_back(value);
  return anka::Word{anka::WordType::DoubleNumber, context.doubleNumbers.size() - 1};
}

auto anka::createWord(Context &context, Tuple &&tuple) -> Word
{
  context.tuples.push_back(std::move(tuple));
  return anka::Word{anka::WordType::Tuple, context.tuples.size() - 1};
}

auto anka::createWord(Context &context, Executor &&executor) -> Word
{
  context.executors.push_back(std::move(executor));
  return anka::Word{anka::WordType::Executor, context.executors.size() - 1};
}

auto anka::createWord(Context &context, Block &&block) -> Word
{
  context.blocks.push_back(std::move(block));
  return anka::Word{anka::WordType::Block, context.blocks.size() - 1};
}

auto anka::createWord(Context &context, std::vector<int> &&vec) -> Word
{
  context.integerArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::IntegerArray, context.integerArrays.size() - 1};
}

auto anka::createWord(Context &context, std::vector<bool> &&vec) -> Word
{
  context.booleanArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::BooleanArray, context.booleanArrays.size() - 1};
}

auto anka::createWord(Context &context, std::vector<double> &&vec) -> Word
{
  context.doubleArrays.push_back(std::move(vec));
  return anka::Word{anka::WordType::DoubleArray, context.doubleArrays.size() - 1};
}

auto anka::getWord(const Context &context, const Word &input, size_t index) -> std::optional<Word>
{
  if (input.type == WordType::Tuple)
  {
    const auto &tup = context.tuples[input.index];
    if (tup.words.size() <= index)
      return std::nullopt;
    return tup.words[index];
  }

  return input;
}

auto anka::getAllWords(const Context &context, const Word &input) -> std::vector<Word>
{
  if (input.type == WordType::Tuple)
  {
    const auto &tup = context.tuples[input.index];
    return tup.words;
  }

  return {input};
}

auto anka::getWordCount(const Context &context, const Word &word) -> size_t
{
  if (word.type != WordType::Tuple)
  {
    return 1;
  }

  return getValue<const Tuple &>(context, word.index).words.size();
}
