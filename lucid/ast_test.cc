#include "lucid/ast.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TypeTest, Size) { EXPECT_EQ(sizeof(Type), 40); }

TEST(ExprTest, Size) { EXPECT_EQ(sizeof(Expr), 32); }

TEST(StmtTest, Size) { EXPECT_EQ(sizeof(Stmt), 40); }

}  // namespace
}  // namespace lucid
