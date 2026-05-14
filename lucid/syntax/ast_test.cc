#include "lucid/syntax/ast.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TypeTest, Size) { EXPECT_EQ(sizeof(Type), 20); }

TEST(ExprTest, Size) { EXPECT_EQ(sizeof(Expr), 24); }

TEST(StmtTest, Size) { EXPECT_EQ(sizeof(Stmt), 36); }

}  // namespace
}  // namespace lucid
