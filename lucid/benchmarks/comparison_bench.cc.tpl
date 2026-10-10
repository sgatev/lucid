// Times {NAME}.lu against {NAME}.cc, as `lucid_cpp_comparison` in
// lucid/benchmarks/comparison.bzl generates it for them.

#include "lucid/benchmarks/comparison.h"
#include "lucid/core/benchmarking/benchmarking.h"

namespace lucid {
namespace {

ComparedProgram program("{PACKAGE}", "{NAME}");

BENCHMARK(Lucid) { program.RunLucid(state); }

BENCHMARK(Cpp) { program.RunCpp(state); }

}  // namespace
}  // namespace lucid
