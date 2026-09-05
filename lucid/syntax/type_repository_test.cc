#include "lucid/syntax/type_repository.h"

#include "lucid/core/container/arena.h"
#include "lucid/core/string/index.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

TEST(Test, TypeRepositoryContainsBuiltins) {
  StringIndex i;
  TypeRepository r(i);

  EXPECT_EQ(r.Size(), 6);
}

TEST(Test, TypeRepositoryResolveBuiltin) {
  StringIndex i;
  TypeRepository r(i);

  TypeRef int64_ref = r.Resolve(i.ref("Int64"));
  EXPECT_NE(int64_ref, Arena<Type>::kNullRef);
  EXPECT_THAT(r.Deref(int64_ref), Variant<BasicType>(Equals(BasicType{
                                      .name = i.ref("Int64"),
                                      .size = 8,
                                  })));
}

TEST(Test, TypeRepositoryResolveMissing) {
  StringIndex i;
  TypeRepository r(i);

  EXPECT_EQ(r.Resolve(i.ref("Foo")), Arena<Type>::kNullRef);
}

TEST(Test, TypeRepositoryResolveCustom) {
  StringIndex i;
  TypeRepository r(i);

  TypeRef foo_ref = r.Add(TupleType{});
  r.Register(i.ref("Foo"), foo_ref);
  EXPECT_EQ(r.Size(), 7);
  EXPECT_EQ(r.Resolve(i.ref("Foo")), foo_ref);
  EXPECT_THAT(r.Deref(foo_ref), Variant<TupleType>(Equals(TupleType{})));
}

}  // namespace
}  // namespace lucid
