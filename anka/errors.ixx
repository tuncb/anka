module;
#include <optional>
#include <string>

#include "ast.h"
export module anka:errors;

namespace anka
{
export struct ExecutionError
{
  std::optional<Word> word1;
  std::optional<Word> word2;

  std::string msg;
};

} // namespace anka
