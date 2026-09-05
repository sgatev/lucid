#include "lucid/arm64/assembler.h"

#include "lucid/core/testing/testing.h"

namespace lucid::arm64 {
namespace {

TEST(Test, InstSize) { EXPECT_EQ(sizeof(Inst), 48); }

}  // namespace
}  // namespace lucid::arm64
