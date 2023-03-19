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

#include <replxx.hxx>

#include <ranges>

#include "ast.h"
#include "executor.h"
#include "tokenizer.h"
#include "internal_functions.h"

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

auto execute(anka::Context &&context, const std::string_view content) -> std::optional<anka::Context>
{
  anka::AST ast;
  try
  {
    auto tokens = anka::extractTokens(content);
    ast = anka::parseAST(content, tokens, std::move(context));
    auto wordOpt = anka::execute(ast);

    if (wordOpt.has_value())
    {
      std::cout << std::format("{}\n", toString(ast.context, wordOpt.value()));
    }

    return ast.context;
  }
  catch (const anka::TokenizerError &err)
  {
    std::cerr << std::format("Token error at {} character: {}.\n", err.pos, err.ch);
    return std::nullopt;
  }
  catch (const anka::ASTError &err)
  {
    std::cerr << std::format("AST error: {}.\n", err.message);
    if (err.tokenOpt.has_value())
    {
      auto t = err.tokenOpt.value();
      std::cerr << std::format("Token start: {}, length: {}.\n", t.token_start, t.len);
    }
    return std::nullopt;
  }
  catch (const anka::ExecutionError &err)
  {
    std::cerr << err.msg << "\n";
    if (err.word1.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(ast.context, err.word1.value()));
    }
    if (err.word2.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(ast.context, err.word2.value()));
    }
    return std::nullopt;
  }
}

auto intrepretRepl(anka::Context &&context) -> void
{
  using Replxx = replxx::Replxx;
  Replxx rx;

  std::cout << "Special functions:\n";
  std::cout << ".exit: Exit REPL.\n";
  std::cout << ".clear: Clear screen.\n";
  std::cout << ".internal: List internal commands and constants.\n";
  std::cout << ".history: List command history.\n";

  std::string prompt = "\x1b[1;32manka\x1b[0m> ";

  for (;;)
  {
    char const *cinput{nullptr};

    do
    {
      cinput = rx.input(prompt);
    } while ((cinput == nullptr) && (errno == EAGAIN));

    if (cinput == nullptr)
    {
      break;
    }

    std::string input{cinput};

    if (input.empty())
    {
      continue;
    }
    else if (input.compare(0, 5, ".exit") == 0)
    {
      // exit the repl
      rx.history_add(input);
      break;
    }
    else if (input.compare(0, 6, ".clear") == 0)
    {
      rx.clear_screen();
      rx.history_add(input);
    }
    else if (input.compare(0, 8, ".history") == 0)
    {
      // display the current history
      Replxx::HistoryScan hs(rx.history_scan());
      for (int i(0); hs.next(); ++i)
      {
        std::cout << std::setw(4) << i << ": " << hs.get().text() << "\n";
      }

      rx.history_add(input);
    }
    else if (input.compare(0, 9, ".internal") == 0)
    {
      const auto& internalFunctions = anka::getInternalFunctions();

      auto kv = std::views::keys(internalFunctions);
      std::vector<std::string> names{kv.begin(), kv.end()};
      std::sort(names.begin(), names.end());

      for (auto name : names)
      {
        std::cout << std::format("{}\n", name);
      }
      rx.history_add(input);
    }
    else
    {
      auto contextOpt = execute(std::move(context), input);
      if (contextOpt.has_value())
      {
        context = std::move(contextOpt.value());
      }
      rx.history_add(input);
    }
  }
}

int main(int argc, char *argv[])
{
  using namespace argumentum;

  std::optional<std::string> filenameOpt;
  auto runInterpreter = false;

  auto parser = argument_parser{};
  auto params = parser.params();
  parser.config().program(argv[0]).description("Anka");
  params.add_parameter(filenameOpt, "--filename", "-f").nargs(1).help("File name to process");
  params.add_parameter(runInterpreter, "--interpreter", "-i").nargs(0).help("Run the interpreter");

  if (!parser.parse_args(argc, argv))
    return -1;

  anka::Context context;

  if (filenameOpt.has_value())
  {
    const auto filename = filenameOpt.value();
    std::cout << "anka: " << std::format("Processing file: {}.\n", filename);
    if (!std::filesystem::exists(filename))
    {
      throw std::runtime_error("File does not exist.");
    }

    auto content = readFile(filename.c_str());

    auto contextOpt = execute(std::move(context), content);
    if (!contextOpt.has_value())
    {
      return -1;
    }
    context = std::move(contextOpt.value());
  }

  if (runInterpreter)
  {
    intrepretRepl(std::move(context));
  }
}

#endif