#ifndef DOCTEST_CONFIG_DISABLE
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#endif

#ifdef DOCTEST_CONFIG_DISABLE

#include <filesystem>
#include <format>

#include <string>

#include <argumentum/argparse.h>
#include <replxx.hxx>

#include <ranges>

#include "ast.h"
#include "executor.h"
#include "internal_functions.h"
#include "tokenizer.h"

import utility;

const auto constexpr MAJOR_VERSION = "0";
const auto constexpr MINOR_VERSION = "2";
const auto constexpr PATCH_VERSION = "0";

auto execute(anka::Context &context, const std::string_view content) -> bool
{
  try
  {
    auto tokens = anka::extractTokens(content);
    auto sentences = anka::parseAST(content, tokens, context);
    auto wordOpt = anka::execute(context, sentences);

    if (wordOpt.has_value())
    {
      std::cout << std::format("{}\n", toString(context, wordOpt.value()));
    }

    return false;
  }
  catch (const anka::TokenizerError &err)
  {
    std::cerr << std::format("Token error at {} character: {}.\n", err.pos, err.ch);
    return false;
  }
  catch (const anka::ASTError &err)
  {
    std::cerr << std::format("AST error: {}.\n", err.message);
    if (err.tokenOpt.has_value())
    {
      auto t = err.tokenOpt.value();
      std::cerr << std::format("Token start: {}, length: {}.\n", t.start, t.len);
    }
    return false;
  }
  catch (const anka::ExecutionError &err)
  {
    std::cerr << err.msg << "\n";
    if (err.word1.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(context, err.word1.value()));
    }
    if (err.word2.has_value())
    {
      std::cerr << std::format("word: {}\n", toString(context, err.word2.value()));
    }
    return false;
  }

  return true;
}

auto executeRepl(anka::Context &context) -> void
{
  using Replxx = replxx::Replxx;
  Replxx rx;

  std::cout << "Special functions:\n";
  std::cout << ".exit: Exit REPL.\n";
  std::cout << ".cls: Clear screen.\n";
  std::cout << ".clear: Clear context.\n";
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
    else if (input == ".cls")
    {
      rx.clear_screen();
      rx.history_add(input);
    }
    else if (input == ".clear")
    {
      context = anka::Context{};
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
      const auto &internalFunctions = anka::getInternalFunctions();

      auto kv = std::views::keys(internalFunctions);
      std::vector<std::string> names{kv.begin(), kv.end()};
      std::sort(names.begin(), names.end());
      auto maxLength =
          std::max_element(names.begin(), names.end(), [](const std::string &name1, const std::string &name2) {
            return name1.length() < name2.length();
          })->length();

      for (auto name : names)
      {
        const auto &overloads = internalFunctions.at(name);
        for (auto &overload : overloads)
        {
          std::cout << std::format("{:<{}}: {}\n", name, maxLength, anka::toString(overload.type));
        }
      }
      rx.history_add(input);
    }
    else
    {
      execute(context, input);
      rx.history_add(input);
    }
  }
}

int main(int argc, char *argv[])
{
  using namespace argumentum;

  const auto appDesc = std::format("anka {}.{}.{}", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);

  std::optional<std::string> filenameOpt;
  auto runRepl = false;

  auto parser = argument_parser{};
  auto params = parser.params();
  parser.config().program(argv[0]).description(appDesc);
  parser.add_default_help_option();
  params.add_parameter(filenameOpt, "--filename", "-f").nargs(1).help("File name to process");
  params.add_parameter(runRepl, "--repl", "-r").nargs(0).help("Run REPL");

  if (!parser.parse_args(argc, argv))
    return -1;

  anka::Context context;
  anka::injectInternalConstants(context);

  std::cout << appDesc << "\n";

  if (!filenameOpt && !runRepl)
  {
    std::cerr << "No argument given, use '-h' to see the vailable options.\n";
  }

  if (filenameOpt)
  {
    const auto filename = filenameOpt.value();
    std::cout << "anka: " << std::format("Processing file: {}.\n", filename);
    if (!std::filesystem::exists(filename))
    {
      throw std::runtime_error("File does not exist.");
    }

    auto content = anka::utility::readFile(filename.c_str());

    if (!execute(context, content))
    {
      return -1;
    }
  }

  if (runRepl)
  {
    executeRepl(context);
  }
}

#endif