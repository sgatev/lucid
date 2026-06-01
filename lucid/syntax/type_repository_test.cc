#include "lucid/syntax/type_repository.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

using ::testing::VariantWith;

TEST(TypeRepositoryTest, ContainsBuiltins) {
  StringIndex i;
  TypeRepository r(i);

  EXPECT_EQ(r.Size(), 6);
}

TEST(TypeRepositoryTest, ResolveBuiltin) {
  StringIndex i;
  TypeRepository r(i);

  TypeRef int64_ref = r.Resolve(i.ref("Int64"));
  EXPECT_NE(int64_ref, Arena<Type>::kNullRef);
  EXPECT_THAT(r.Deref(int64_ref), VariantWith<BasicType>(BasicType{
                                      .name = i.ref("Int64"),
                                      .size = 8,
                                  }));
}

TEST(TypeRepositoryTest, ResolveMissing) {
  StringIndex i;
  TypeRepository r(i);

  EXPECT_EQ(r.Resolve(i.ref("Foo")), Arena<Type>::kNullRef);
}

TEST(TypeRepositoryTest, ResolveCustom) {
  StringIndex i;
  TypeRepository r(i);

  TypeRef foo_ref = r.Add(BasicType{.name = i.ref("Foo"), .size = 4});
  EXPECT_EQ(r.Size(), 7);
  EXPECT_EQ(r.Resolve(i.ref("Foo")), foo_ref);
  EXPECT_THAT(r.Deref(foo_ref), VariantWith<BasicType>(BasicType{
                                    .name = i.ref("Foo"),
                                    .size = 4,
                                }));
}

}  // namespace
}  // namespace lucid
