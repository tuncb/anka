#ifndef DOCTEST_CONFIG_DISABLE
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#endif

#ifdef DOCTEST_CONFIG_DISABLE

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include <argumentum/argparse.h>

#include "ast.h"
#include "executor.h"
#include "tokenizer.h"

auto readFile(const char *filename) -> std::string
{
  std::ifstream t(filename);
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  return str;
}

auto executeFile(const std::string &filename) -> void
{
  std::cout << "anka: " << std::format("Processing file: {}.\n", filename);
  if (!std::filesystem::exists(filename))
  {
    throw std::runtime_error("File does not exist.");
  }

  auto content = readFile(filename.c_str());
  try
  {
    anka::Context context;
    auto tokens = anka::extractTokens(content);
    auto ast = anka::parseAST(content, tokens, std::move(context));
    auto wordOpt = anka::execute(ast);

    if (wordOpt.has_value())
    {
      std::cout << std::format("Result: {}\n", toString(ast.context, wordOpt.value()));
    }
    else
    {
      std::cout << "No result.\n";
    }
  }
  catch (const anka::TokenizerError &err)
  {
    std::cerr << std::format("Token error at {} character: {}.\n", err.pos, err.ch);
  }
  catch (const anka::ASTError &err)
  {
    std::cerr << std::format("AST error: {}.\n", err.message);
    if (err.tokenOpt.has_value())
    {
      auto t = err.tokenOpt.value();
      std::cerr << std::format("Token start: {}, length: {}.\n", t.token_start, t.len);
    }
  }
  catch (const anka::ExecutionError &err)
  {
    std::cerr << err.msg;
    if (err.word1.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(err.context, err.word1.value()));
    }
    if (err.word2.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(err.context, err.word2.value()));
    }
  }
}

int main(int argc, char *argv[])
{
  using namespace argumentum;

  std::optional<std::string> filenameOpt;

  auto parser = argument_parser{};
  auto params = parser.params();
  parser.config().program(argv[0]).description("Anka");
  params.add_parameter(filenameOpt, "--filename", "-f").nargs(1).help("File name to process");

  if (!parser.parse_args(argc, argv))
    return -1;

  if (filenameOpt.has_value())
  {
    executeFile(filenameOpt.value());
  }
  else
  {
    std::cerr << "A filename is needed to process.";
    return -1;
  }
}

#endif