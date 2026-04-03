#include "lucid/arm64/assembler.h"

#include "gtest/gtest.h"

namespace lucid::arm64 {
namespace {

TEST(InstTest, Size) { EXPECT_EQ(sizeof(Inst), 48); }

}  // namespace
}  // namespace lucid::arm64
