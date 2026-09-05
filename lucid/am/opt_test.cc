#include "lucid/am/opt.h"

#include <list>

#include "lucid/am/instructions.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, OptimizeAbstractMachineInstructionsRemovesUnnecessaryInstructions) {
  std::list<Instruction> instructions = {
      MoveReg{
          .src_reg = Reg(1, RegSize32),
          .dst_reg = Reg(2, RegSize32),
      },
      MoveReg{
          .src_reg = Reg(3, RegSize32),
          .dst_reg = Reg(3, RegSize32),
      },
      AddReg{
          .res_reg = Reg(1, RegSize32),
          .lhs_reg = Reg(2, RegSize32),
          .rhs_reg = Reg(2, RegSize32),
      },
  };

  OptimizeAbstractMachineInstructions(instructions);

  EXPECT_THAT(instructions, ElementsEqual(
                                MoveReg{
                                    .src_reg = Reg(1, RegSize32),
                                    .dst_reg = Reg(2, RegSize32),
                                },
                                Nop{},
                                AddReg{
                                    .res_reg = Reg(1, RegSize32),
                                    .lhs_reg = Reg(2, RegSize32),
                                    .rhs_reg = Reg(2, RegSize32),
                                }));
}

}  // namespace
}  // namespace lucid
