#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

bool Test::RunFull() {
  failed_ = false;
  Run();
  return !failed_;
}

void Test::Fail(std::string_view reason) {
  std::cout << "Fail: " << reason << "\n";
  failed_ = true;
}

static std::vector<std::unique_ptr<lucid::Test>> kRegisteredTests;

int AddTest(std::unique_ptr<Test> test) {
  kRegisteredTests.push_back(std::move(test));
  return 1;
}

// Runs all tests in the global suite, prints a summary to stdout, and
// returns the number of tests that failed.
int RunAllTests(std::vector<std::string_view> args) {
  bool enable_timings = true;
  if (args.size() > 1 && args[1] == "--disable_timings") {
    enable_timings = false;
  }

  struct TestResult {
    std::string_view name;
    bool pass;
    std::chrono::microseconds elapsed_time;
    std::stringstream output;
  };

  std::vector<TestResult> test_results;
  test_results.reserve(kRegisteredTests.size());
  for (const auto& test : kRegisteredTests) {
    test_results.push_back(TestResult{.name = test->Name()});
  }

  const auto suite_start_time = std::chrono::system_clock::now();
  for (int i = 0; i < kRegisteredTests.size(); ++i) {
    std::streambuf* original_cout_buf = std::cout.rdbuf();
    std::cout.rdbuf(test_results[i].output.rdbuf());

    const auto start_time = std::chrono::system_clock::now();
    const bool pass = kRegisteredTests[i]->RunFull();
    const auto end_time = std::chrono::system_clock::now();
    const auto elapsed_time = end_time - start_time;

    std::cout.rdbuf(original_cout_buf);

    test_results[i].pass = pass;
    test_results[i].elapsed_time = elapsed_time;
  }
  const auto suite_end_time = std::chrono::system_clock::now();
  const auto suite_elapsed_time = suite_end_time - suite_start_time;

  for (const auto& test_result : test_results) {
    if (test_result.pass) {
      std::cout << "│ PASS " << test_result.name;

    } else {
      std::cout << "│ FAIL " << test_result.name;
    }
    if (enable_timings) {
      std::cout << " (" << test_result.elapsed_time << ")";
    }
    std::cout << "\n";
  }

  const std::size_t passed_tests_count = std::ranges::count_if(
      test_results, [](const auto& test_result) { return test_result.pass; });
  const std::size_t failed_tests_count = std::ranges::count_if(
      test_results, [](const auto& test_result) { return !test_result.pass; });
  std::cout << "└─► " << passed_tests_count << " out of "
            << kRegisteredTests.size() << " tests PASS";
  if (enable_timings) {
    std::cout << "(" << suite_elapsed_time << ")";
  }
  std::cout << "\n";

  std::cout << "\n";

  for (const auto& test_result : test_results) {
    if (test_result.pass) continue;

    std::cout << "== " << test_result.name << " ==\n";
    std::cout << test_result.output.str() << "\n";
  }

  return static_cast<int>(failed_tests_count);
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::RunAllTests({argv, argv + argc});
}
