#include "lucid/core/numeric/bits.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, TwosComplement7Works) {
  EXPECT_EQ(TwosComplement7(0b00000000), 0b00000000);
  EXPECT_EQ(TwosComplement7(0b00000001), 0b01111111);
  EXPECT_EQ(TwosComplement7(0b01111111), 0b00000001);
}

}  // namespace
}  // namespace lucid
