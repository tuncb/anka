module;
// https://developercommunity.visualstudio.com/t/Template-exports-requiring-importing-of-/1425979
#include <compare>
export module anka;
export import :internal_functions;
export import :errors;
export import :type_system;
export import :interpreter_state;
export import :state_utilities;
export import :executor;
export import :tokenizer;
export import :parser;