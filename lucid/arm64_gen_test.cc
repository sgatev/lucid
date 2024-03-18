#include "lucid/arm64_gen.h"

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
  std::string Generate(
      FuncDefStmt& func,
      const std::unordered_map<std::string_view, FuncType>& func_types = {}) {
    InferExprTypes(stmt_arena_, expr_arena_, type_arena_, func_types, func);
    auto graph = BuildControlFlowGraph(stmt_arena_, expr_arena_, func);
    AbstractMachineState state;
    GenerateAbstractMachineFunction(stmt_arena_, expr_arena_, type_arena_,
                                    graph, state);
    std::stringstream out;
    GenerateArmAssemblySource(state.func, out);
    return out.str();
  }
};

TEST_F(GenerateArmAssemblySourceTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(IntLitExpr{.value = "21"}),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #21
MOV W0, W1
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(IntLitExpr{.value = "21"}),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #21
MOV X0, X1
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt32Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = T(BasicType{.name = "Int32"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(IdentExpr{
                      .name = "x",
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(id:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR W1, [SP, #0]
id0:
LDR W1, [SP, #0]
MOV W0, W1
B id1
id1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt64Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = T(BasicType{.name = "Int64"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(IdentExpr{
                      .name = "x",
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(id:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
id0:
LDR X1, [SP, #0]
MOV X0, X1
B id1
id1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithInt32Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(FuncCallExpr{
                      .func_name = "id",
                      .args = ExprListOf(IntLitExpr{.value = "21"}),
                  }),
              }),
          },
  };

  std::vector<FuncParam> id_func_params = {
      {.type = T(BasicType{.name = "Int32"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_EQ(Generate(func, {{"id", id_func_type}}), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #96
STR X12, [SP, #88]
STR X11, [SP, #80]
STR X10, [SP, #72]
STR X9, [SP, #64]
STR X8, [SP, #56]
STR X7, [SP, #48]
STR X6, [SP, #40]
STR X5, [SP, #32]
STR X4, [SP, #24]
STR X3, [SP, #16]
STR X2, [SP, #8]
STR X1, [SP, #0]
foo0:
MOV W1, #21
MOV W1, W1
BL id
MOV W2, W0
MOV W0, W2
B foo1
foo1:
LDR X12, [SP, #88]
LDR X11, [SP, #80]
LDR X10, [SP, #72]
LDR X9, [SP, #64]
LDR X8, [SP, #56]
LDR X7, [SP, #48]
LDR X6, [SP, #40]
LDR X5, [SP, #32]
LDR X4, [SP, #24]
LDR X3, [SP, #16]
LDR X2, [SP, #8]
LDR X1, [SP, #0]
ADD SP, SP, #96
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithInt64Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(FuncCallExpr{
                      .func_name = "id",
                      .args = ExprListOf(IntLitExpr{.value = "21"}),
                  }),
              }),
          },
  };

  std::vector<FuncParam> id_func_params = {
      {.type = T(BasicType{.name = "Int64"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int64",
      .parameters = id_func_params,
  };

  EXPECT_EQ(Generate(func, {{"id", id_func_type}}), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #96
STR X12, [SP, #88]
STR X11, [SP, #80]
STR X10, [SP, #72]
STR X9, [SP, #64]
STR X8, [SP, #56]
STR X7, [SP, #48]
STR X6, [SP, #40]
STR X5, [SP, #32]
STR X4, [SP, #24]
STR X3, [SP, #16]
STR X2, [SP, #8]
STR X1, [SP, #0]
foo0:
MOV X1, #21
MOV X1, X1
BL id
MOV X2, X0
MOV X0, X2
B foo1
foo1:
LDR X12, [SP, #88]
LDR X11, [SP, #80]
LDR X10, [SP, #72]
LDR X9, [SP, #64]
LDR X8, [SP, #56]
LDR X7, [SP, #48]
LDR X6, [SP, #40]
LDR X5, [SP, #32]
LDR X4, [SP, #24]
LDR X3, [SP, #16]
LDR X2, [SP, #8]
LDR X1, [SP, #0]
ADD SP, SP, #96
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Add,
                      .lhs = E(IntLitExpr{.value = "2"}),
                      .rhs = E(IntLitExpr{.value = "3"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #2
MOV W2, #3
ADD W3, W1, W2
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Add,
                      .lhs = E(IntLitExpr{.value = "2"}),
                      .rhs = E(IntLitExpr{.value = "3"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #2
MOV X2, #3
ADD X3, X1, X2
MOV X0, X3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Sub,
                      .lhs = E(IntLitExpr{.value = "7"}),
                      .rhs = E(IntLitExpr{.value = "5"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #7
MOV W2, #5
SUB W3, W1, W2
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Sub,
                      .lhs = E(IntLitExpr{.value = "7"}),
                      .rhs = E(IntLitExpr{.value = "5"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #7
MOV X2, #5
SUB X3, X1, X2
MOV X0, X3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Mul,
                      .lhs = E(IntLitExpr{.value = "2"}),
                      .rhs = E(IntLitExpr{.value = "3"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #2
MOV W2, #3
MUL W3, W1, W2
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Mul,
                      .lhs = E(IntLitExpr{.value = "2"}),
                      .rhs = E(IntLitExpr{.value = "3"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #2
MOV X2, #3
MUL X3, X1, X2
MOV X0, X3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Div,
                      .lhs = E(IntLitExpr{.value = "8"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #8
MOV W2, #2
UDIV W3, W1, W2
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Div,
                      .lhs = E(IntLitExpr{.value = "8"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #8
MOV X2, #2
UDIV X3, X1, X2
MOV X0, X3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, ModuloInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Mod,
                      .lhs = E(IntLitExpr{.value = "8"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #8
MOV W2, #2
UDIV W3, W1, W2
MSUB W3, W3, W2, W1
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, ModuloInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Mod,
                      .lhs = E(IntLitExpr{.value = "8"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV X1, #8
MOV X2, #2
UDIV X3, X1, X2
MSUB X3, X3, X2, X1
MOV X0, X3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, IfStmt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body = {
          .stmts = StmtListOf(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_body = {
                  .stmts = StmtListOf(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = "2"}),
                          .rhs = E(IntLitExpr{.value = "3"}),
                      }),
                  }),
              },
          },
          ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = "4"}),
                  .rhs = E(IntLitExpr{.value = "5"}),
              }),
          }),
      },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #1
CMP W1, 0
B.EQ foo2
B foo3
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
foo2:
MOV W1, #4
MOV W2, #5
MUL W3, W1, W2
MOV W0, W3
B foo1
foo3:
MOV W1, #2
MOV W2, #3
ADD W3, W1, W2
MOV W0, W3
B foo1
)");
}

TEST_F(GenerateArmAssemblySourceTest, IfElseStmt) {
  auto
      func =
          FuncDefStmt{
              .name = "foo",
              .result_type = T(BasicType{.name = "Int32"}),
              .body =
                  {
                      .stmts =
                          StmtListOf(
                              IfStmt{
                                  .cond = E(BoolLitExpr{.value = "true"}),
                                  .then_body =
                                      {
                                          .stmts =
                                              StmtListOf(
                                                  ReturnStmt{
                                                      .value =
                                                          E(
                                                              BinaryOpExpr{
                                                                  .op =
                                                                      BinaryOp::Add,
                                                                  .lhs =
                                                                      E(
                                                                          IntLitExpr{
                                                                              .value = "2"}),
                                                                  .rhs =
                                                                      E(
                                                                          IntLitExpr{
                                                                              .value = "3"}),
                                                              }),
                                                  }),
                                      },
                                  .else_body =
                                      {
                                          .stmts =
                                              StmtListOf(
                                                  ReturnStmt{
                                                      .value =
                                                          E(
                                                              BinaryOpExpr{
                                                                  .op =
                                                                      BinaryOp::Mul,
                                                                  .lhs =
                                                                      E(
                                                                          IntLitExpr{
                                                                              .value = "4"}),
                                                                  .rhs =
                                                                      E(
                                                                          IntLitExpr{
                                                                              .value = "5"}),
                                                              }),
                                                  }),
                                      },
                              }),
                  },
          };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #1
CMP W1, 0
B.EQ foo4
B foo3
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
foo2:
B foo1
foo3:
MOV W1, #2
MOV W2, #3
ADD W3, W1, W2
MOV W0, W3
B foo1
foo4:
MOV W1, #4
MOV W2, #5
MUL W3, W1, W2
MOV W0, W3
B foo1
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Gt,
                      .lhs = E(IntLitExpr{.value = "3"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, GT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Gt,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR W1, [SP, #0]
STR W2, [SP, #4]
foo0:
LDR W1, [SP, #0]
LDR W2, [SP, #4]
CMP W1, W2
CSET W3, GT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Gt,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
CMP X1, X2
CSET X3, GT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Lt,
                      .lhs = E(IntLitExpr{.value = "3"}),
                      .rhs = E(IntLitExpr{.value = "2"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #0
foo0:
MOV W1, #3
MOV W2, #2
CMP W1, W2
CSET W3, LT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #0
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Lt,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR W1, [SP, #0]
STR W2, [SP, #4]
foo0:
LDR W1, [SP, #0]
LDR W2, [SP, #4]
CMP W1, W2
CSET W3, LT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Lt,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
CMP X1, X2
CSET X3, LT
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Eq,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
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
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::Eq,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
CMP X1, X2
CSET X3, EQ
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, NotEqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::NotEq,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR W1, [SP, #0]
STR W2, [SP, #4]
foo0:
LDR W1, [SP, #0]
LDR W2, [SP, #4]
CMP W1, W2
CSET W3, NE
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, NotEqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(BinaryOpExpr{
                      .op = BinaryOp::NotEq,
                      .lhs = E(IdentExpr{.name = "x"}),
                      .rhs = E(IdentExpr{.name = "y"}),
                  }),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
CMP X1, X2
CSET X3, NE
MOV W0, W3
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf(
                  VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int32"}),
                      .init = E(IntLitExpr{.value = "2"}),
                  },
                  VarDeclStmt{
                      .name = "y",
                      .type = T(BasicType{.name = "Int32"}),
                      .init = E(IntLitExpr{.value = "3"}),
                  },
                  ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{
                              .name = "x",
                          }),
                          .rhs = E(IdentExpr{
                              .name = "y",
                          }),
                      }),
                  }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
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
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int64"}),
      .body =
          {
              .stmts = StmtListOf(
                  VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int64"}),
                      .init = E(IntLitExpr{.value = "2"}),
                  },
                  VarDeclStmt{
                      .name = "y",
                      .type = T(BasicType{.name = "Int64"}),
                      .init = E(IntLitExpr{.value = "3"}),
                  },
                  ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{
                              .name = "x",
                          }),
                          .rhs = E(IdentExpr{
                              .name = "y",
                          }),
                      }),
                  }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
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
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarAssignInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(VarAssignStmt{
                  .name = "x",
                  .expr = E(IntLitExpr{.value = "2"}),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR W1, [SP, #0]
foo0:
MOV W1, #2
STR W1, [SP, #0]
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarAssignInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(VarAssignStmt{
                  .name = "x",
                  .expr = E(IntLitExpr{.value = "2"}),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
foo0:
MOV X1, #2
STR X1, [SP, #0]
B foo1
foo1:
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, Printf) {
  auto func = FuncDefStmt{
      .name = "printf",
      .result_type = T(BasicType{.name = "Int32"}),
      .parameters =
          {
              {
                  .name = "f",
                  .type = T(BasicType{.name = "String"}),
              },
              {
                  .name = "n",
                  .type = T(BasicType{.name = "Int64"}),
              },
              {
                  .name = "m",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf(ReturnStmt{
                  .value = E(IntLitExpr{.value = "0"}),
              }),
          },
  };

  EXPECT_EQ(Generate(func), R"(printf:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X2, [SP, #0]
STR X3, [SP, #8]
MOV X0, X1
BL _printf
MOV W0, #0
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

}  // namespace
}  // namespace lucid
