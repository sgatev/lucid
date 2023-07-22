#include "lucid/cpp_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

TEST(GenerateCppSourceTest, SimpleFunctionDefinition) {
  Arena<Stmt> arena;
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void foo() {
};
)");
}

TEST(GenerateCppSourceTest, FunctionWithOneStatement) {
  Arena<Stmt> arena;
  auto int_lit_ref = arena.add(IntLit{.value = 21});
  auto return_stmt_ref = arena.add(ReturnStmt{.value = int_lit_ref});
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {return_stmt_ref},
          },
      .result_type = "int",
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(int foo() {
  return 21;
};
)");
}

TEST(GenerateCppSourceTest, ReturnFunctionCall) {
  Arena<Stmt> arena;
  auto func_call_stmt_ref = arena.add(FuncCallExpr{.func_name = "bar"});
  auto return_stmt_ref = arena.add(ReturnStmt{.value = func_call_stmt_ref});
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {return_stmt_ref},
          },
      .result_type = "int",
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(int foo() {
  return bar();
};
)");
}

TEST(GenerateCppSourceTest, VariableDeclaration) {
  Arena<Stmt> arena;
  auto func_call_stmt_ref = arena.add(FuncCallExpr{.func_name = "bar"});
  auto x_var_decl_ref = arena.add(VarDeclStmt{
      .type = "int",
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {x_var_decl_ref},
          },
      .result_type = "void",
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void foo() {
  int x = bar();
};
)");
}

TEST(GenerateCppSourceTest, FunctionWithParameters) {
  Arena<Stmt> arena;
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
      .parameters =
          {
              FuncParam{
                  .type = "int",
                  .name = "x",
              },
              FuncParam{
                  .type = "int",
                  .name = "y",
              },
          },
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void foo(int x, int y) {
};
)");
}

TEST(GenerateCppSourceTest, ReturnIdentifier) {
  Arena<Stmt> arena;
  auto ident_expr_ref = arena.add(IdentExpr{
      .name = "x",
  });
  auto return_stmt_ref = arena.add(ReturnStmt{.value = ident_expr_ref});
  auto func_stmt_ref = arena.add(FuncDefStmt{
      .name = "id",
      .result_type = "void",
      .parameters =
          {
              FuncParam{
                  .type = "int",
                  .name = "x",
              },
          },
      .body =
          {
              .statements =
                  {
                      return_stmt_ref,
                  },
          },
  });
  EXPECT_EQ(GenerateCppSource(arena, arena.get(func_stmt_ref)),
            R"(void id(int x) {
  return x;
};
)");
}

}  // namespace
}  // namespace lucid
