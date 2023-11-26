#include "lucid/arm64_gen.h"

#include <string>
#include <string_view>
#include <strstream>
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
    InferExprTypes(arena_, func_types, func);
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
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(IntLitExpr{.value = "21"}),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
foo0:
MOV W1, #21
MOV W0, W1
B foo1
foo1:
LDR X1, [SP, #0]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(IntLitExpr{.value = "21"}),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
foo0:
MOV X1, #21
MOV X0, X1
B foo1
foo1:
LDR X1, [SP, #0]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt32Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = A(BasicType{.name = "Int32"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(IdentExpr{
                  .name = "x",
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(id:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncWithInt64Param) {
  auto func = FuncDefStmt{
      .name = "id",
      .result_type = A(BasicType{.name = "Int64"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(IdentExpr{
                  .name = "x",
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(id:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithInt32Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(FuncCallExpr{
                  .func_name = "id",
                  .arguments =
                      {
                          A(IntLitExpr{.value = "21"}),
                      },
              }),
          }),
      }},
  };

  std::vector<FuncParam> id_func_params = {
      {.type = A(BasicType{.name = "Int32"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_EQ(Generate(func, {{"id", id_func_type}}), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
MOV W1, #21
MOV W1, W1
BL id
MOV W2, W0
MOV W0, W2
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithInt64Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(FuncCallExpr{
                  .func_name = "id",
                  .arguments =
                      {
                          A(IntLitExpr{.value = "21"}),
                      },
              }),
          }),
      }},
  };

  std::vector<FuncParam> id_func_params = {
      {.type = A(BasicType{.name = "Int64"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int64",
      .parameters = id_func_params,
  };

  EXPECT_EQ(Generate(func, {{"id", id_func_type}}), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #0]
STR X2, [SP, #8]
foo0:
MOV X1, #21
MOV X1, X1
BL id
MOV X2, X0
MOV X0, X2
B foo1
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = A(IntLitExpr{.value = "7"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = A(IntLitExpr{.value = "7"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = A(IntLitExpr{.value = "8"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = A(IntLitExpr{.value = "8"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, IfStmt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(IfStmt{
              .cond = A(BoolLitExpr{.value = "true"}),
              .then_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = A(IntLitExpr{.value = "2"}),
                          .rhs = A(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }},
          }),
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "4"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
B.EQ foo2
B foo3
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
LDR X4, [SP, #24]
LDR X5, [SP, #32]
LDR X6, [SP, #40]
LDR X7, [SP, #48]
ADD SP, SP, #64
LDP X29, X30, [SP], #16
RET
foo2:
MOV W2, #4
MOV W3, #5
MUL W4, W2, W3
MOV W0, W4
B foo1
foo3:
MOV W5, #2
MOV W6, #3
ADD W7, W5, W6
MOV W0, W7
B foo1
)");
}

TEST_F(GenerateArmAssemblySourceTest, IfElseStmt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(IfStmt{
              .cond = A(BoolLitExpr{.value = "true"}),
              .then_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = A(IntLitExpr{.value = "2"}),
                          .rhs = A(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }},
              .else_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = A(IntLitExpr{.value = "4"}),
                          .rhs = A(IntLitExpr{.value = "5"}),
                      }),
                  }),
              }},
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
B.EQ foo4
B foo3
foo1:
LDR X1, [SP, #0]
LDR X2, [SP, #8]
LDR X3, [SP, #16]
LDR X4, [SP, #24]
LDR X5, [SP, #32]
LDR X6, [SP, #40]
LDR X7, [SP, #48]
ADD SP, SP, #64
LDP X29, X30, [SP], #16
RET
foo2:
B foo1
foo3:
MOV W2, #2
MOV W3, #3
ADD W4, W2, W3
MOV W0, W4
B foo1
foo4:
MOV W5, #4
MOV W6, #5
MUL W7, W5, W6
MOV W0, W7
B foo1
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IntLitExpr{.value = "3"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET W3, GT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #8]
LDR X2, [SP, #16]
LDR X3, [SP, #24]
ADD SP, SP, #32
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET X3, GT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
ADD SP, SP, #48
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IntLitExpr{.value = "3"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET W3, LT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #8]
LDR X2, [SP, #16]
LDR X3, [SP, #24]
ADD SP, SP, #32
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET X3, LT
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
ADD SP, SP, #48
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
ADD SP, SP, #48
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, NotEqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET W3, NE
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #8]
LDR X2, [SP, #16]
LDR X3, [SP, #24]
ADD SP, SP, #32
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, NotEqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
CSET X3, NE
MOV W0, W3
B foo1
foo1:
LDR X1, [SP, #16]
LDR X2, [SP, #24]
LDR X3, [SP, #32]
ADD SP, SP, #48
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(VarDeclStmt{
              .name = "x",
              .type = A(BasicType{.name = "Int32"}),
              .init = A(IntLitExpr{.value = "2"}),
          }),
          A(VarDeclStmt{
              .name = "y",
              .type = A(BasicType{.name = "Int32"}),
              .init = A(IntLitExpr{.value = "3"}),
          }),
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IdentExpr{
                      .name = "x",
                  }),
                  .rhs = A(IdentExpr{
                      .name = "y",
                  }),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(VarDeclStmt{
              .name = "x",
              .type = A(BasicType{.name = "Int64"}),
              .init = A(IntLitExpr{.value = "2"}),
          }),
          A(VarDeclStmt{
              .name = "y",
              .type = A(BasicType{.name = "Int64"}),
              .init = A(IntLitExpr{.value = "3"}),
          }),
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IdentExpr{
                      .name = "x",
                  }),
                  .rhs = A(IdentExpr{
                      .name = "y",
                  }),
              }),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
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
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarAssignInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(VarAssignStmt{
              .name = "x",
              .expr = A(IntLitExpr{.value = "2"}),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #4]
STR W1, [SP, #0]
foo0:
MOV W1, #2
STR W1, [SP, #0]
B foo1
foo1:
LDR X1, [SP, #4]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, VarAssignInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(VarAssignStmt{
              .name = "x",
              .expr = A(IntLitExpr{.value = "2"}),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(foo:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X1, [SP, #8]
STR X1, [SP, #0]
foo0:
MOV X1, #2
STR X1, [SP, #0]
B foo1
foo1:
LDR X1, [SP, #8]
ADD SP, SP, #16
LDP X29, X30, [SP], #16
RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, Printf) {
  auto func = FuncDefStmt{
      .name = "printf",
      .result_type = A(BasicType{.name = "Int32"}),
      .parameters =
          {
              {
                  .name = "n",
                  .type = A(BasicType{.name = "String"}),
              },
              {
                  .name = "m",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(IntLitExpr{.value = "0"}),
          }),
      }},
  };

  EXPECT_EQ(Generate(func), R"(printf:
STP X29, X30, [SP, #-16]!
SUB SP, SP, #16
STR X2, [SP, #0]
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
