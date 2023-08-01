#include "lucid/arm_assembly_gen.h"

#include <string>

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

  std::string Generate(FuncDefStmt func) {
    return GenerateArmAssemblySource(arena_, func);
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateArmAssemblySourceTest, FunctionWithOneStatement) {
  auto func = FuncDefStmt{
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

  EXPECT_EQ(Generate(func), R"(.global main
.align 2
main:
  mov X1, #21
  mov X0, X1
  mov X16, #1
  svc #0x80
)");
}

}  // namespace
}  // namespace lucid
