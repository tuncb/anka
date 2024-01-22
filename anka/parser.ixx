module;
#include <algorithm>
#include <cassert>
#include <format>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>
#include <span>

export module anka:parser;

import :tokenizer;
import :interpreter_state;

template <class T>
concept TokenForwardIterator =
    std::forward_iterator<T> && std::is_same_v<typename std::iterator_traits<T>::value_type, anka::Token>;

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

// Forward declerations
auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd, std::optional<size_t> connectedNameOpt) -> anka::Word;
auto extractArray(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;
auto extractExecutor(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                     TokenForwardIterator auto tokensEnd) -> anka::Word;
auto extractBlock(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;

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

namespace anka
{
export auto parseAST(const std::string_view content, std::span<Token> tokens, Context &context) -> std::vector<Sentence>
{
  std::vector<Sentence> sentences;
  for (auto tokenIter = tokens.begin(); tokenIter != tokens.end();)
  {
    sentences.emplace_back(extractSentence(content, context, tokenIter, tokens.end()));
  }

  return sentences;
}
} // namespace anka