module;
#include <optional>
#include <string>

export module anka:errors;

import :interpreter_state;

namespace anka
{
export struct ExecutionError
{
  std::optional<Word> word1;
  std::optional<Word> word2;

  std::string msg;
};

} // namespace anka
