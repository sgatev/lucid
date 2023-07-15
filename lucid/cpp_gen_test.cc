#include "lucid/cpp_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

TEST(GenerateCppSourceTest, SimpleFunctionDefinition) {
  Arena<Stmt> arena;
  auto func_stmt_ref = arena.add(FuncDefStmt{.name = "foo"});
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void foo() {
}
)");
}

TEST(GenerateCppSourceTest, FunctionWithOneStatement) {
  Arena<Stmt> arena;
  auto return_stmt_ref = arena.add(ReturnStmt());
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {return_stmt_ref},
          },
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void foo() {
  return;
}
)");
}

}  // namespace
}  // namespace lucid
