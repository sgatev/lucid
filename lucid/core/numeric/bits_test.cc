#include "lucid/core/numeric/bits.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, TwosComplement7Works) {
  EXPECT_EQ(TwosComplement7(0b00000000), 0b00000000);
  EXPECT_EQ(TwosComplement7(0b00000001), 0b01111111);
  EXPECT_EQ(TwosComplement7(0b01111111), 0b00000001);
}

TEST(Test, TwosComplement9Works) {
  EXPECT_EQ(TwosComplement9(0b000000000), 0b000000000);
  EXPECT_EQ(TwosComplement9(0b000000001), 0b111111111);
  EXPECT_EQ(TwosComplement9(0b011111111), 0b100000001);
  EXPECT_EQ(TwosComplement9(0b111111111), 0b000000001);
}

}  // namespace
}  // namespace lucid
