module;

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>

#include <string>
#include <variant>
#include <vector>
export module anka:type_system;

namespace anka
{

// helper type for the visitor #4
template <class... Ts> struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

export enum class TypeFamily
{
  Void,
  Bool,
  Int,
  Double,
  BoolArray,
  IntArray,
  DoubleArray,
};

export struct FunctionType
{
  TypeFamily returnType;
  std::vector<TypeFamily> arguments;

  bool operator==(const FunctionType &rhs) const
  {
    return arguments == rhs.arguments;
  }
};

export using TypeVariant = std::variant<TypeFamily, FunctionType>;
export using TypeList = std::vector<TypeVariant>;

export inline auto isExpandable(const TypeVariant &type) -> bool
{
  if (type.index() != 0)
    return false;
  switch (std::get<TypeFamily>(type))
  {
  case TypeFamily::Bool:
  case TypeFamily::Int:
  case TypeFamily::Double:
    return true;
  default:
    return false;
  }
}

export bool operator==(const TypeVariant &v1, const TypeVariant &v2)
{
  if (v1.index() != v2.index())
    return false;

  return std::visit(overloaded{[&v2](const FunctionType &val) { return std::get<FunctionType>(v2) == val; },
                               [&v2](const TypeFamily &val) { return std::get<TypeFamily>(v2) == val; }},
                    v1);
}

struct TypeHasher
{
  inline auto operator()(const TypeFamily type) const -> size_t
  {
    return std::hash<int>()(static_cast<int>(type));
  }

  inline auto operator()(const FunctionType funcType) const -> size_t
  {
    auto ret = funcType.arguments.size();
    for (auto type : funcType.arguments)
    {
      ret = ret ^ (*this)(type);
    }

    return ret;
  }
};

struct TypeStringConvertor
{
  inline auto operator()(const TypeFamily type) const -> std::string
  {
    switch (type)
    {
    case TypeFamily::Void:
      return "void";
    case TypeFamily::Bool:
      return "bool";
    case TypeFamily::Int:
      return "int";
    case TypeFamily::Double:
      return "double";
    case TypeFamily::BoolArray:
      return "(bool)";
    case TypeFamily::IntArray:
      return "(int)";
    case TypeFamily::DoubleArray:
      return "(double)";
    default:
      return "unknown";
    }
  }

  inline auto operator()(const FunctionType &funcType) const -> std::string
  {

    std::vector<std::string> argText = funcType.arguments |
                                       ranges::views::transform([this](const TypeFamily t) { return (*this)(t); }) |
                                       ranges::to<std::vector>();

    return fmt::format("[{}] -> {}", fmt::join(argText, " "), (*this)(funcType.returnType));
  }
};

export auto hash(const TypeList &types) -> size_t
{
  size_t ret = 0;
  TypeHasher hasher;

  for (auto type : types)
  {
    ret = ret ^ std::visit(hasher, type);
  }
  return ret;
}

export auto toString(const TypeList &types) -> std::vector<std::string>
{
  TypeStringConvertor conv;

  std::vector<std::string> argText =
      types | ranges::views::transform([conv](const TypeVariant &v) { return std::visit(conv, v); }) |
      ranges::to<std::vector>();

  return argText;
}

export template <typename T1, typename T2>
concept IsSameType = std::is_same_v<typename std::remove_cv<typename std::remove_reference<T1>::type>::type, T2>;

export template <typename T>
concept IsTypeFamilyCompatible =
    IsSameType<T, int> || IsSameType<T, double> || IsSameType<T, bool> || IsSameType<T, std::vector<bool>> ||
    IsSameType<T, std::vector<int>> || IsSameType<T, std::vector<double>>;

template <IsTypeFamilyCompatible T> auto getFamilyType() -> TypeFamily
{
  using Decayed = std::remove_cv<typename std::remove_reference<T>::type>::type;

  if constexpr (std::is_same_v<Decayed, int>)
    return TypeFamily::Int;
  else if constexpr (std::is_same_v<Decayed, std::vector<int>>)
    return TypeFamily::IntArray;
  else if constexpr (std::is_same_v<Decayed, bool>)
    return TypeFamily::Bool;
  else if constexpr (std::is_same_v<Decayed, std::vector<bool>>)
    return TypeFamily::BoolArray;
  else if constexpr (std::is_same_v<Decayed, double>)
    return TypeFamily::Double;
  else if constexpr (std::is_same_v<Decayed, std::vector<double>>)
    return TypeFamily::DoubleArray;
  else
    []<bool flag = false>()
    {
      static_assert(flag, "No match found in function getValue().");
    }
  ();
}

template <IsTypeFamilyCompatible T> auto getValueType() -> TypeVariant
{
  auto f = getFamilyType<T>();
  return f;
}

template <typename ReturnType, typename... Args> auto getFunctionType(std::function<ReturnType(Args...)>) -> TypeVariant
{
  FunctionType func;
  func.returnType = getFamilyType<ReturnType>();
  func.arguments = std::vector<TypeFamily>{getFamilyType<Args>()...};

  return func;
}

export template <typename T> auto getType() -> TypeVariant
{
  using FuncType = std::remove_pointer<T>::type;
  if constexpr (std::is_function_v<FuncType>)
  {
    return getFunctionType(std::function<FuncType>());
  }
  else
  {
    return getValueType<T>();
  }
}

} // namespace anka
