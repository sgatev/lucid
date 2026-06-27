#include "lucid/syntax/type_repository.h"

#include <string_view>
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
    StringIndex::Ref name = idents_.ref(type.name);
    TypeRef ref = Add(BasicType{.name = name, .size = type.size});
    Register(name, ref);
  }
}

TypeRef TypeRepository::Add(Type type) { return types_.Add(type); }

const Type& TypeRepository::Deref(TypeRef ref) const { return types_.Get(ref); }

void TypeRepository::Register(StringIndex::Ref name, TypeRef ref) {
  name_to_type_.Insert(name, ref);
}

TypeRef TypeRepository::Resolve(StringIndex::Ref name) const {
  return name_to_type_.Get(name).value_or(Arena<Type>::kNullRef);
}

std::size_t TypeRepository::Size() const { return types_.Size(); }

}  // namespace lucid
