#include "lucid/core/hash/hash.h"

#include <cstdint>
#include <string_view>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

std::size_t H(std::uint32_t v) { return Hash(v); }

TEST(Test, HashCombineDependsOnTheOrderOfItsHashes) {
  EXPECT_NE(HashCombine(H(1), H(2)), HashCombine(H(2), H(1)));
  EXPECT_NE(HashCombine(H(1), H(2), H(3)), HashCombine(H(3), H(2), H(1)));
}

TEST(Test, HashCombineKeepsHashesThatRepeat) {
  // Folding with exclusive or alone would cancel these to nothing, so a value
  // carrying one field twice over would hash as though it carried neither.
  EXPECT_NE(HashCombine(H(1), H(1)), 0);
  EXPECT_NE(HashCombine(H(1), H(2), H(3), H(3)), HashCombine(H(1), H(2)));
  EXPECT_NE(HashCombine(H(1), H(1)), HashCombine(H(2), H(2)));
}

TEST(Test, HashOfEqualStringsIsEqual) {
  EXPECT_EQ(Hash(std::string_view("lucid")), Hash(std::string_view("lucid")));
  EXPECT_NE(Hash(std::string_view("lucid")), Hash(std::string_view("lucifer")));
}

}  // namespace
}  // namespace lucid
