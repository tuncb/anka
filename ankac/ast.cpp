#include "ast.h"

#include <charconv>
#include <format>
#include <iterator>

#include <fmt/ranges.h>

template <class T>
concept TokenForwardIterator =
    std::forward_iterator<T> && std::is_same_v<typename std::iterator_traits<T>::value_type, anka::Token>;

auto toInt(const std::string_view content, anka::Token token) -> int
{
  auto b = &(content[token.token_start]);
  auto e = b + token.len;
  int value = 0;
  auto res = std::from_chars(b, e, value);
  if (res.ec == std::errc{} && res.ptr == e)
  {
    return value;
  }

  auto msg = std::format("Expected integer number, found: {}.", std::string(b, e));
  throw anka::ASTError({token, msg});
}

auto addIntegerWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
{
  auto token = *(tokenIter++);
  auto value = toInt(content, token);
  context.integerNumbers.push_back(value);
  return {anka::WordType::IntegerNumber, context.integerNumbers.size() - 1};
}

auto addNameWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter)
    -> anka::Word
{
  auto token = *(tokenIter++);
  auto value = std::string(content.data() + token.token_start, content.data() + token.token_start + token.len);
  context.names.push_back(value);
  return {anka::WordType::Name, context.names.size() - 1};
}

auto addArrayWord(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word
{
  using namespace anka;

  ++tokenIter;
  if (tokenIter == tokensEnd)
  {
    throw ASTError{std::nullopt, "Array ended unexpectedly."};
  }

  if (tokenIter->type == TokenType::ArrayEnd)
  {
    throw ASTError{*tokenIter, "Empty arrays are not supported."};
  }

  if (tokenIter->type != TokenType::NumberInt)
  {
    throw ASTError{*tokenIter, "Only integer number arrays are supported."};
  }

  const auto tokenType = TokenType::NumberInt;

  std::vector<int> numbers;

  for (; tokenIter != tokensEnd;)
  {
    if (tokenIter->type == TokenType::ArrayEnd)
    {
      tokenIter++;
      context.integerArrays.emplace_back(std::move(numbers));
      return {anka::WordType::IntegerArray, context.integerArrays.size() - 1};
    }

    if (tokenIter->type != tokenType)
    {
      throw ASTError{*tokenIter, "Unexpected token type."};
    }

    numbers.push_back(toInt(content, *tokenIter));
    tokenIter++;
  }

  throw ASTError{std::nullopt, "Array ended unexpectedly."};
}

auto extractSentence(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                     TokenForwardIterator auto tokensEnd) -> anka::Sentence
{
  using namespace anka;

  Sentence sentence;
  for (; tokenIter != tokensEnd;)
  {
    switch (tokenIter->type)
    {
    case TokenType::NumberInt:
      sentence.words.emplace_back(addIntegerWord(content, context, tokenIter));
      break;
    case TokenType::Name:
      sentence.words.emplace_back(addNameWord(content, context, tokenIter));
      break;
    case TokenType::ArrayStart:
      sentence.words.emplace_back(addArrayWord(content, context, tokenIter, tokensEnd));
      break;
    case TokenType::ArrayEnd:
      throw ASTError(*tokenIter, "Did not expect to find an array end token.");
    case TokenType::SentenceEnd:
      ++tokenIter;
      return sentence;
    }
  }

  return sentence;
}

auto anka::parseAST(const std::string_view content, std::span<Token> tokens, Context &&context) -> AST
{
  std::vector<Sentence> sentences;
  for (auto tokenIter = tokens.begin(); tokenIter != tokens.end();)
  {
    sentences.emplace_back(extractSentence(content, context, tokenIter, tokens.end()));
  }

  return {std::move(context), sentences};
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
  case WordType::Name:
    return context.names[word.index];
  default:
    return "";
  }
}
