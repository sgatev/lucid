#include "lucid/cpp_gen.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

class GenerateCppSourceTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  std::string Generate(StmtRef ref) {
    return GenerateCppSource(arena_, arena_.get(ref));
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateCppSourceTest, SimpleFunctionDefinition) {
  auto func_stmt_ref = Allocate(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
  });
  EXPECT_EQ(Generate(func_stmt_ref), R"(void foo() {
};
)");
}

TEST_F(GenerateCppSourceTest, FunctionWithOneStatement) {
  auto int_lit_ref = Allocate(IntLit{.value = 21});
  auto return_stmt_ref = Allocate(ReturnStmt{.value = int_lit_ref});
  auto func_stmt_ref = Allocate(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {return_stmt_ref},
          },
      .result_type = "int",
  });
  EXPECT_EQ(Generate(func_stmt_ref), R"(int foo() {
  return 21;
};
)");
}

TEST_F(GenerateCppSourceTest, ReturnFunctionCall) {
  auto func_call_stmt_ref =
      Allocate(FuncCallExpr{.func_name = "bar",
                            .arguments = {
                                Allocate(IntLit{.value = 3}),
                                Allocate(IntLit{.value = 7}),
                            }});
  auto return_stmt_ref = Allocate(ReturnStmt{.value = func_call_stmt_ref});
  auto func_stmt_ref = Allocate(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {return_stmt_ref},
          },
      .result_type = "int",
  });
  EXPECT_EQ(Generate(func_stmt_ref), R"(int foo() {
  return bar(3, 7);
};
)");
}

TEST_F(GenerateCppSourceTest, VariableDeclaration) {
  auto func_call_stmt_ref = Allocate(FuncCallExpr{.func_name = "bar"});
  auto x_var_decl_ref = Allocate(VarDeclStmt{
      .type = "int",
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto func_stmt_ref = Allocate(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {x_var_decl_ref},
          },
      .result_type = "void",
  });
  EXPECT_EQ(Generate(func_stmt_ref), R"(void foo() {
  int x = bar();
};
)");
}

TEST_F(GenerateCppSourceTest, FunctionWithParameters) {
  auto func_stmt_ref = Allocate(FuncDefStmt{
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
  EXPECT_EQ(Generate(func_stmt_ref), R"(void foo(int x, int y) {
};
)");
}

TEST_F(GenerateCppSourceTest, ReturnIdentifier) {
  auto ident_expr_ref = Allocate(IdentExpr{
      .name = "x",
  });
  auto return_stmt_ref = Allocate(ReturnStmt{.value = ident_expr_ref});
  auto func_stmt_ref = Allocate(FuncDefStmt{
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
  EXPECT_EQ(Generate(func_stmt_ref), R"(void id(int x) {
  return x;
};
)");
}

}  // namespace
}  // namespace lucid
