#include "lucid/syntax/type_repository.h"

#include <optional>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/core/container/arena.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {

TypeRepository::TypeRepository(StringIndex& idents) : idents_(idents) {
  static constexpr struct {
    std::string_view name;
    std::size_t size;
  } kBuiltinTypes[] = {
      {.name = "Void", .size = 0},   {.name = "Bool", .size = 4},
      {.name = "Int32", .size = 4},  {.name = "Int64", .size = 8},
      {.name = "Double", .size = 8}, {.name = "String", .size = 8},
  };
  for (const auto& type : kBuiltinTypes) {
    Add(BasicType{.name = idents_.ref(type.name), .size = type.size});
  }
}

TypeRef TypeRepository::Add(Type type) {
  std::optional<StringIndex::Ref> name;
  if (const auto* basic_type = std::get_if<BasicType>(&type)) {
    name = basic_type->name;
  }
  TypeRef ref = types_.Add(std::move(type));
  if (name.has_value()) name_to_type_.Insert(*name, ref);
  return ref;
}

TypeRef TypeRepository::Resolve(StringIndex::Ref name) const {
  return name_to_type_.Get(name).value_or(Arena<Type>::kNullRef);
}

const Type& TypeRepository::Deref(TypeRef ref) const { return types_.Get(ref); }

std::size_t TypeRepository::Size() const { return types_.Size(); }

}  // namespace lucid
