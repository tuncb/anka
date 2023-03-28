module;
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
export module utility;

namespace anka::utility
{

export auto readFile(const char *filename) -> std::string
{
  std::ifstream t(filename);
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  return str;
}

} // namespace anka::utility
