#include "lucid/arm64_gen.h"

#include <string>
#include <string_view>
#include <strstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

class GenerateArmAssemblySourceTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  std::string Generate(const FuncDefStmt& func) {
    auto graph = BuildControlFlowGraph(arena_, func);
    auto instructions = GenerateAbstractMachineInstructions(arena_, graph);
    std::strstream out;
    GenerateArmAssemblySource(func.name, instructions, out);
    return out.str();
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateArmAssemblySourceTest, ReturnIntLit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IntLitExpr{.value = "21"}),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #21
MOV X0, X1
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithArg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(FuncCallExpr{
                              .func_name = "id",
                              .arguments =
                                  {
                                      Allocate(IntLitExpr{.value = "21"}),
                                  },
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #21
STP X29, X30, [sp, #-16]!
BL id
LDP X29, X30, [sp], #16
MOV X0, X0
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #2
MOV X2, #3
ADD X3, X1, X2
MOV X0, X3
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Sub,
                              .lhs = Allocate(IntLitExpr{.value = "7"}),
                              .rhs = Allocate(IntLitExpr{.value = "5"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #7
MOV X2, #5
SUB X3, X1, X2
MOV X0, X3
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Mul,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #2
MOV X2, #3
MUL X3, X1, X2
MOV X0, X3
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Div,
                              .lhs = Allocate(IntLitExpr{.value = "8"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #8
MOV X2, #2
UDIV X3, X1, X2
MOV X0, X3
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, IfStmt) {
  auto return_add_expr = Allocate(ReturnStmt{
      .value = Allocate(BinaryOpExpr{
          .op = BinaryOp::Add,
          .lhs = Allocate(IntLitExpr{.value = "2"}),
          .rhs = Allocate(IntLitExpr{.value = "3"}),
      }),
  });
  auto return_mul_expr = Allocate(ReturnStmt{
      .value = Allocate(BinaryOpExpr{
          .op = BinaryOp::Mul,
          .lhs = Allocate(IntLitExpr{.value = "4"}),
          .rhs = Allocate(IntLitExpr{.value = "5"}),
      }),
  });
  auto if_stmt = Allocate(IfStmt{
      .cond = Allocate(BoolLitExpr{.value = "true"}),
      .then_body = {.statements =
                        {
                            return_add_expr,
                        }},
      .else_body = {.statements =
                        {
                            return_mul_expr,
                        }},
  });
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      if_stmt,
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV X1, #1
CMP X1, 0
B.EQ foo2
B.NE foo1
foo1:
MOV X2, #2
MOV X3, #3
ADD X4, X2, X3
MOV X0, X4
RET
foo2:
MOV X5, #4
MOV X6, #5
MUL X7, X5, X6
MOV X0, X7
RET
)");
}

}  // namespace
}  // namespace lucid
