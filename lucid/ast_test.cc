#include "lucid/ast.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TypeTest, Size) { EXPECT_EQ(sizeof(Type), 40); }

TEST(ExprTest, Size) { EXPECT_EQ(sizeof(Expr), 48); }

TEST(StmtTest, Size) { EXPECT_EQ(sizeof(Stmt), 72); }

}  // namespace
}  // namespace lucid
