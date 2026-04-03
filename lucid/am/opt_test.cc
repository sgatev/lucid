#include "lucid/am/opt.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(OptimizeAbstractMachineInstructionsTest, RemovesUnnecessaryInstructions) {
  std::vector<Instruction> instructions = {
      MoveReg32{.src_reg = 1, .dst_reg = 2},
      MoveReg32{.src_reg = 3, .dst_reg = 3},
      AddReg32{.res_reg = 1, .lhs_reg = 2, .rhs_reg = 2},
  };

  OptimizeAbstractMachineInstructions(instructions);

  EXPECT_THAT(instructions, ElementsAre(
                                MoveReg32{
                                    .src_reg = 1,
                                    .dst_reg = 2,
                                },
                                Nop{},
                                AddReg32{
                                    .res_reg = 1,
                                    .lhs_reg = 2,
                                    .rhs_reg = 2,
                                }));
}

}  // namespace
}  // namespace lucid
