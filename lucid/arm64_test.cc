#include "lucid/arm64.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid::arm64 {
namespace {

TEST(InstTest, Size) { EXPECT_EQ(sizeof(Inst), 24); }

}  // namespace
}  // namespace lucid::arm64
