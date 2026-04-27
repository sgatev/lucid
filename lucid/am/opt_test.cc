#include "lucid/am/opt.h"

#include <list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(OptimizeAbstractMachineInstructionsTest, RemovesUnnecessaryInstructions) {
  std::list<Instruction> instructions = {
      MoveReg{
          .src_reg = RegId(1, RegSize32),
          .dst_reg = RegId(2, RegSize32),
      },
      MoveReg{
          .src_reg = RegId(3, RegSize32),
          .dst_reg = RegId(3, RegSize32),
      },
      AddReg{
          .res_reg = RegId(1, RegSize32),
          .lhs_reg = RegId(2, RegSize32),
          .rhs_reg = RegId(2, RegSize32),
      },
  };

  OptimizeAbstractMachineInstructions(instructions);

  EXPECT_THAT(instructions, ElementsAre(
                                MoveReg{
                                    .src_reg = RegId(1, RegSize32),
                                    .dst_reg = RegId(2, RegSize32),
                                },
                                Nop{},
                                AddReg{
                                    .res_reg = RegId(1, RegSize32),
                                    .lhs_reg = RegId(2, RegSize32),
                                    .rhs_reg = RegId(2, RegSize32),
                                }));
}

}  // namespace
}  // namespace lucid
