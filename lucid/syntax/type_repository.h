#pragma once

#include <cstddef>

#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {

// A repository of types.
class TypeRepository {
 public:
  explicit TypeRepository(StringIndex& idents);

  // Adds a type to the repository.
  TypeRef Add(Type type);

  // Returns a reference to the type that `name` resolves to.
  TypeRef Resolve(StringIndex::Ref name) const;

  // Returns the type that `ref` refers to.
  const Type& Deref(TypeRef ref) const;

  // Returns the number of types in the repository.
  std::size_t Size() const;

 private:
  StringIndex& idents_;
  Arena<Type> types_;
  HashMap<StringIndex::Ref, TypeRef> name_to_type_;
};

}  // namespace lucid
