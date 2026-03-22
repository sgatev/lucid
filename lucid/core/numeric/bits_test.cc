#include "lucid/core/numeric/bits.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TwosComplement7Test, Works) {
  EXPECT_EQ(TwosComplement7(0b00000000), 0b00000000);
  EXPECT_EQ(TwosComplement7(0b00000001), 0b01111111);
  EXPECT_EQ(TwosComplement7(0b01111111), 0b00000001);
}

}  // namespace
}  // namespace lucid
