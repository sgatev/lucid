#include "lucid/am_gen.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

class GenerateAbstractMachineInstructionsTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  std::vector<Instruction> Generate(const FuncDefStmt& func) {
    auto graph = BuildControlFlowGraph(arena_, func);
    return GenerateAbstractMachineInstructions(arena_, graph);
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateAbstractMachineInstructionsTest, ReturnIntLit) {
  auto main_func = FuncDefStmt{
      .name = "main",
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

  EXPECT_THAT(Generate(main_func), ElementsAre(
                                       SetReg32{
                                           .src_val = "21",
                                           .dst_reg = 1,
                                       },
                                       MoveReg32{
                                           .src_reg = 1,
                                           .dst_reg = 0,
                                       },
                                       Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, FuncCallWithArg) {
  auto main_func = FuncDefStmt{
      .name = "main",
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

  EXPECT_THAT(Generate(main_func), ElementsAre(
                                       SetReg32{
                                           .src_val = "21",
                                           .dst_reg = 1,
                                       },
                                       Jump{
                                           .label = "id",
                                       },
                                       MoveReg32{
                                           .src_reg = 0,
                                           .dst_reg = 0,
                                       },
                                       Return{}));
}

}  // namespace
}  // namespace lucid
