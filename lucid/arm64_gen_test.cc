#include "lucid/arm64_gen.h"

#include <string>
#include <string_view>
#include <strstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"
#include "lucid/cfg.h"
#include "lucid/type.h"

namespace lucid {
namespace {

class GenerateArmAssemblySourceTest : public testing::Test, public AstFixture {
 protected:
  std::string Generate(FuncDefStmt& func) {
    InferExpressionTypes(arena_, func);
    auto graph = BuildControlFlowGraph(arena_, func);
    AbstractMachineState state;
    GenerateAbstractMachineFunction(arena_, graph, state);
    std::strstream out;
    GenerateArmAssemblySource(state.func, out);
    return out.str();
  }
};

TEST_F(GenerateArmAssemblySourceTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #16
STR X1, [SP, #0]
foo0:
MOV W1, #21
MOV W0, W1
B foo1
foo1:
LDR X1, [SP, #0]
ADD SP, SP, #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #16
STR X1, [SP, #0]
foo0:
MOV X1, #21
MOV X0, X1
B foo1
foo1:
LDR X1, [SP, #0]
ADD SP, SP, #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt32Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = "Int32",
      .parameters =
          {
              {
                  .name = "x",
                  .type = "Int32",
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(id:
SUB SP, SP, #16
STR X1, [SP, #4]
STR W1, [SP, #0]
id0:
LDR W1, [SP, #0]
MOV W0, W1
B id1
id1:
LDR X1, [SP, #4]
ADD SP, SP, #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt64Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = "Int64",
      .parameters =
          {
              {
                  .name = "x",
                  .type = "Int64",
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(id:
SUB SP, SP, #16
STR X1, [SP, #8]
STR X1, [SP, #0]
id0:
LDR X1, [SP, #0]
MOV X0, X1
B id1
id1:
LDR X1, [SP, #8]
ADD SP, SP, #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithArg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
MOV W1, #21
MOV W1, W1
STP X29, X30, [sp, #-16]!
BL id
LDP X29, X30, [sp], #16
MOV W2, W0
MOV W0, W2
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
ADD SP, SP, #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #2
MOV W2, #3
ADD W3, W1, W2
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #2
MOV X2, #3
ADD X3, X1, X2
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #7
MOV W2, #5
SUB W3, W1, W2
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #7
MOV X2, #5
SUB X3, X1, X2
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #2
MOV W2, #3
MUL W3, W1, W2
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #2
MOV X2, #3
MUL X3, X1, X2
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #8
MOV W2, #2
UDIV W3, W1, W2
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #8
MOV X2, #2
UDIV X3, X1, X2
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
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
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      if_stmt,
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
SUB SP, SP, #64
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
STR X4, [SP, #24]
STR X5, [SP, #32]
STR X6, [SP, #40]
STR X7, [SP, #48]
foo0:
MOV W1, #1
CMP W1, 0
B.EQ foo3
B foo2
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
LDR X4, [SP, #24]
LDR X5, [SP, #32]
LDR X6, [SP, #40]
LDR X7, [SP, #48]
ADD SP, SP, #64
RET
foo2:
MOV W2, #2
MOV W3, #3
ADD W4, W2, W3
MOV W0, W4
B foo1
foo3:
MOV W5, #4
MOV W6, #5
MUL W7, W5, W6
MOV W0, W7
B foo1
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, GT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #3
MOV X2, #2
CMP X1, X2
CSET X3, GT
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, LT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
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
SUB SP, SP, #32
STR X1, [SP, #0]
STR X2, [SP, #8]
STR X3, [SP, #16]
foo0:
MOV X1, #3
MOV X2, #2
CMP X1, X2
CSET X3, LT
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .parameters =
          {
              {
                  {
                      .name = "x",
                      .type = "Int32",
                  },
                  {
                      .name = "y",
                      .type = "Int32",
                  },
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Eq,
                              .lhs = Allocate(IdentExpr{.name = "x"}),
                              .rhs = Allocate(IdentExpr{.name = "y"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
SUB SP, SP, #32
STR X1, [SP, #8]
STR X2, [SP, #16]
STR X3, [SP, #24]
STR W1, [SP, #0]
STR W2, [SP, #4]
foo0:
LDR W1, [SP, #0]
LDR W2, [SP, #4]
CMP W1, W2
CSET W3, EQ
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #8]
LDR X2, [SP, #16]
LDR X3, [SP, #24]
ADD SP, SP, #32
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .parameters =
          {
              {
                  {
                      .name = "x",
                      .type = "Int64",
                  },
                  {
                      .name = "y",
                      .type = "Int64",
                  },
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Eq,
                              .lhs = Allocate(IdentExpr{.name = "x"}),
                              .rhs = Allocate(IdentExpr{.name = "y"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
SUB SP, SP, #48
STR X1, [SP, #16]
STR X2, [SP, #24]
STR X3, [SP, #32]
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
CMP X1, X2
CSET X3, EQ
MOV X0, X3
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
ADD SP, SP, #48
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int32",
                          .init = Allocate(IntLitExpr{.value = "2"}),
                      }),
                      Allocate(VarDeclStmt{
                          .name = "y",
                          .type = "Int32",
                          .init = Allocate(IntLitExpr{.value = "3"}),
                      }),
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IdentExpr{
                                  .name = "x",
                              }),
                              .rhs = Allocate(IdentExpr{
                                  .name = "y",
                              }),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
SUB SP, SP, #48
STR X1, [SP, #8]
STR X2, [SP, #16]
STR X3, [SP, #24]
STR X4, [SP, #32]
STR X5, [SP, #40]
foo0:
MOV W1, #2
STR W1, [SP, #0]
MOV W2, #3
STR W2, [SP, #4]
LDR W3, [SP, #0]
LDR W4, [SP, #4]
ADD W5, W3, W4
MOV W0, W5
B foo1
foo1:
LDR X1, [SP, #8]
LDR X2, [SP, #16]
LDR X3, [SP, #24]
LDR X4, [SP, #32]
LDR X5, [SP, #40]
ADD SP, SP, #48
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int64",
                          .init = Allocate(IntLitExpr{.value = "2"}),
                      }),
                      Allocate(VarDeclStmt{
                          .name = "y",
                          .type = "Int64",
                          .init = Allocate(IntLitExpr{.value = "3"}),
                      }),
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IdentExpr{
                                  .name = "x",
                              }),
                              .rhs = Allocate(IdentExpr{
                                  .name = "y",
                              }),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
SUB SP, SP, #64
STR X1, [SP, #16]
STR X2, [SP, #24]
STR X3, [SP, #32]
STR X4, [SP, #40]
STR X5, [SP, #48]
foo0:
MOV X1, #2
STR X1, [SP, #0]
MOV X2, #3
STR X2, [SP, #8]
LDR X3, [SP, #0]
LDR X4, [SP, #8]
ADD X5, X3, X4
MOV X0, X5
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
LDR X4, [SP, #40]
LDR X5, [SP, #48]
ADD SP, SP, #64
RET
)");
}

}  // namespace
}  // namespace lucid
