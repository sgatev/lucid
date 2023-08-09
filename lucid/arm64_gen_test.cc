#include "lucid/arm64_gen.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/writer.h"

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
    std::string result;
    GenerateArmAssemblySource(func.name, instructions, StringWriter(result));
    return result;
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
mov X1, #21
mov X0, X1
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
mov X1, #21
stp X29, X30, [sp, #-16]!
BL id
ldp X29, X30, [sp], #16
mov X0, X0
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
mov X1, #2
mov X2, #3
ADD X3, X1, X2
mov X0, X3
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
mov X1, #7
mov X2, #5
SUB X3, X1, X2
mov X0, X3
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
mov X1, #2
mov X2, #3
MUL X3, X1, X2
mov X0, X3
RET
)");
}

}  // namespace
}  // namespace lucid
