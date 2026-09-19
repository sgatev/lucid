#include "lucid/syntax/ast.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, TypeSize) { EXPECT_EQ(sizeof(Type), 32); }

TEST(Test, ExprSize) { EXPECT_EQ(sizeof(Expr), 32); }

TEST(Test, StmtSize) { EXPECT_EQ(sizeof(Stmt), 32); }

}  // namespace
}  // namespace lucid
