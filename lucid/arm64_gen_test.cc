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
    AbstractMachineState state;
    GenerateAbstractMachineInstructions(arena_, graph, state);
    std::strstream out;
    GenerateArmAssemblySource(func.name, state.instructions, out);
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
MOV W1, #21
MOV W0, W1
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
MOV W1, #21
STP X29, X30, [sp, #-16]!
BL id
LDP X29, X30, [sp], #16
MOV W0, W0
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
MOV W1, #2
MOV W2, #3
ADD W3, W1, W2
MOV W0, W3
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
MOV W1, #7
MOV W2, #5
SUB W3, W1, W2
MOV W0, W3
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
MOV W1, #2
MOV W2, #3
MUL W3, W1, W2
MOV W0, W3
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
MOV W1, #8
MOV W2, #2
UDIV W3, W1, W2
MOV W0, W3
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
MOV W1, #1
CMP W1, 0
B.EQ foo2
B.NE foo1
foo1:
MOV W2, #2
MOV W3, #3
ADD W4, W2, W3
MOV W0, W4
RET
foo2:
MOV W5, #4
MOV W6, #5
MUL W7, W5, W6
MOV W0, W7
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Gt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, GT
MOV W0, W3
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInts) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Lt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, LT
MOV W0, W3
RET
)");
}

}  // namespace
}  // namespace lucid
