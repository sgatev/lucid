#include "lucid/am/instructions.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(InstructionTest, Size) { EXPECT_EQ(sizeof(Instruction), 64); }

}  // namespace
}  // namespace lucid
