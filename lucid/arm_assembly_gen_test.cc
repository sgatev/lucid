#include "lucid/arm_assembly_gen.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

class GenerateArmAssemblySourceTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  std::string Generate(const std::vector<FuncDefStmt>& funcs) {
    return GenerateArmAssemblySource(arena_, funcs);
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateArmAssemblySourceTest, ReturnIntLit) {
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

  EXPECT_EQ(Generate({main_func}), R"(.global _start
.align 2
_start:
  stp X29, X30, [sp, #-16]!
  BL main
  ldp X29, X30, [sp], #16
  mov X16, #1
  svc #0x80
main:
  mov X1, #21
  mov X0, X1
  RET
)");
}

TEST_F(GenerateArmAssemblySourceTest, FuncCallWithArg) {
  auto id_func = FuncDefStmt{
      .name = "id",
      .result_type = "int",
      .parameters =
          {
              {
                  .type = "int",
                  .name = "x",
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IdentExpr{.name = "x"}),
                      }),
                  },
          },
  };
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

  EXPECT_EQ(Generate({id_func, main_func}), R"(.global _start
.align 2
_start:
  stp X29, X30, [sp, #-16]!
  BL main
  ldp X29, X30, [sp], #16
  mov X16, #1
  svc #0x80
id:
  mov X0, X1
  RET
main:
  mov X1, #21
  stp X29, X30, [sp, #-16]!
  BL id
  ldp X29, X30, [sp], #16
  mov X0, X0
  RET
)");
}

}  // namespace
}  // namespace lucid
