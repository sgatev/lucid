#include "lucid/syntax/ast.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, TypeSize) { EXPECT_EQ(sizeof(Type), 24); }

TEST(Test, ExprSize) { EXPECT_EQ(sizeof(Expr), 28); }

TEST(Test, StmtSize) { EXPECT_EQ(sizeof(Stmt), 36); }

}  // namespace
}  // namespace lucid
