#include "ast.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <iterator>

#include <fmt/ranges.h>

template <class T>
concept TokenForwardIterator =
    std::forward_iterator<T> && std::is_same_v<typename std::iterator_traits<T>::value_type, anka::Token>;

// Forward declerations
auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;
auto extractArray(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word;

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

  auto msg = std::format("Expected integer number, found: {}", std::string(b, e));
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
      words.emplace_back(addIntegerWord(content, context, tokenIter));
      break;
    case TokenType::Name:
      words.emplace_back(addNameWord(content, context, tokenIter));
      break;
    case TokenType::ArrayStart:
      ++tokenIter;
      words.emplace_back(extractArray(content, context, tokenIter, tokensEnd));
      break;
    case TokenType::TupleStart:
      ++tokenIter;
      words.emplace_back(extractTuple(content, context, tokenIter, tokensEnd));
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

auto extractTuple(const std::string_view content, anka::Context &context, TokenForwardIterator auto &tokenIter,
                  TokenForwardIterator auto tokensEnd) -> anka::Word
{
  using namespace anka;

  auto words = extractWords(content, context, tokenIter, tokensEnd, {TokenType::SentenceEnd, TokenType::ArrayEnd},
                            TokenType::TupleEnd);
  return createWord(context, std::move(words));
};

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
  if (!(expectedType == WordType::IntegerNumber or expectedType == WordType::Boolean))
  {
    throw ASTError{startToken, "Only boolean or integer arrays are supported"};
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
  case WordType::Boolean:
    return std::format("{}", context.booleans[word.index]);
  case WordType::BooleanArray: {
    auto &v = context.booleanArrays[word.index];
    return fmt::format("({})", fmt::join(v, " "));
  }
  case WordType::Tuple: {
    std::vector<std::string> names;
    auto &&tup = context.tuples[word.index];
    std::transform(tup.begin(), tup.end(), std::back_inserter(names),
                   [&context](const Word &word) { return toString(context, word); });
    return fmt::format("[{}]", fmt::join(names, " "));
  }
  case WordType::Name:
    return context.names[word.index];
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

auto anka::createWord(Context &context, std::vector<anka::Word> &&vec) -> Word
{
  context.tuples.push_back(std::move(vec));
  return anka::Word{anka::WordType::Tuple, context.tuples.size() - 1};
}

auto anka::toString(TokenType type) -> std::string
{
  switch (type)
  {
  case TokenType::NumberInt:
    return "integer number";
  case TokenType::Name:
    return "name";
  case TokenType::ArrayStart:
    return "array start '('";
  case TokenType::ArrayEnd:
    return "array end ')'";
  case TokenType::TupleStart:
    return "tuple start '['";
  case TokenType::TupleEnd:
    return "tuple end ']'";
  case TokenType::SentenceEnd:
    return "sentence end";
  default:
    throw(std::runtime_error("Fatal error: Unexpected token type"));
  }
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

auto anka::getWord(const Context &context, const Word &input, size_t index) -> std::optional<Word>
{
  if (input.type == WordType::Tuple)
  {
    const auto &tup = context.tuples[input.index];
    if (tup.size() <= index)
      return std::nullopt;
    return tup[index];
  }

  return input;
}