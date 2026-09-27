#include <cstdint>
#include <format>

#include "lucid/compiler/compiler_test_fixture.h"
#include "lucid/compiler/random_program.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

// How many programs each run works through. Every one of them is compiled
// and run as its own process, so this is what the run costs.
constexpr std::uint64_t kProgramCount = 60;

// Programs the compiler has not been shown, run against what building them
// says they come to.
//
// The tests written by hand say what the compiler does with the shapes
// someone thought of. These say what it does with shapes nobody did, which
// is where the mistakes that matter have been: a slot of the frame standing
// over a register, an immediate too wide for the field carrying it, a value
// crossing a branch with nowhere to be held. Each of those was found this
// way, and none of them by a test written to look for it.
TEST(CompilerTest, RunsRandomProgramsForWhatTheyComeTo) {
  for (std::uint64_t seed = 0; seed < kProgramCount; ++seed) {
    RandomProgram::Program program = RandomProgram(seed).Generate();

    ASSERT_TRUE(CreateFile("main.lu", program.source));
    const CommandResult result = RunCompiler({"run", FullPath("main.lu")});

    // The source is printed where they disagree, because the seed alone
    // says nothing about what the program was.
    if (result.return_code != program.exit_code) {
      std::cout << std::format("seed {} returned {}, not {}:\n{}\n", seed,
                               result.return_code, program.exit_code,
                               program.source);
    }
    EXPECT_THAT(result, ReturnsCode(program.exit_code));
  }
}

}  // namespace
}  // namespace lucid
