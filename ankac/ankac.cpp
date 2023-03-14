#ifndef DOCTEST_CONFIG_DISABLE
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#endif

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include "tokenizer.h"

std::string readFile(const char *filename)
{
  std::ifstream t(filename);
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  return str;
}

#ifdef DOCTEST_CONFIG_DISABLE
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "anka: expected a filename to process.\n";
    return -1;
  }

  const auto filename = std::string(argv[1]);
  std::cout << "anka: " << std::format("Processing file: {}.\n", filename);

  if (!std::filesystem::exists(filename))
  {
    std::cerr << std::format("anka: I could not find file: {}.\n", filename);
    return -1;
  }

  auto content = readFile(filename.c_str());

  try
  {
    auto tokens = anka::extractTokens(content);
    std::cout << std::format("Parsed {} tokens.\n", tokens.size());
  }
  catch (anka::TokenizerError &err)
  {
    std::cerr << std::format("Token error at {} character: {}.\n", err.pos, err.ch);
  }
}
#endif