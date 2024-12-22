#include "lucid/am.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(InstructionTest, Size) { EXPECT_EQ(sizeof(Instruction), 32); }

}  // namespace
}  // namespace lucid
