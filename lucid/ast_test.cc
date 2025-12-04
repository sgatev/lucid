#include "lucid/ast.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TypeTest, Size) { EXPECT_EQ(sizeof(Type), 24); }

TEST(ExprTest, Size) { EXPECT_EQ(sizeof(Expr), 28); }

TEST(StmtTest, Size) { EXPECT_EQ(sizeof(Stmt), 32); }

}  // namespace
}  // namespace lucid
