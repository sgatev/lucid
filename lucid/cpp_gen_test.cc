#include "lucid/cpp_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

TEST(GenerateCppSourceTest, SimpleFunctionDefinition) {
  FuncDefStmt stmt = {.name = "foo"};
  EXPECT_EQ(GenerateCppSource(stmt), R"(void foo() {})");
}

}  // namespace
}  // namespace lucid
