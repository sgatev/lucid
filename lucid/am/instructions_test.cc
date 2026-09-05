#include "lucid/am/instructions.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, InstructionSize) { EXPECT_EQ(sizeof(Instruction), 64); }

}  // namespace
}  // namespace lucid
