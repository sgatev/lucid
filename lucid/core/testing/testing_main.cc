#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/core/cli/cli.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

static std::vector<std::unique_ptr<lucid::Test>> kRegisteredTests;

// Runs all tests in the global suite, prints a summary, and returns the number
// of tests that failed.
int HandleRunTestsCommand(CommandContext ctx) {
  const bool enable_timings = !ctx.Flag("disable_timings").has_value();

  const std::optional<std::string_view> filter = ctx.Flag("filter");
  if (filter.has_value() && filter->empty()) {
    ctx.Err() << "the 'filter' flag requires a value\n";
    return 1;
  }

  std::vector<Test*> selected_tests;
  selected_tests.reserve(kRegisteredTests.size());
  for (const auto& test : kRegisteredTests) {
    if (!filter.has_value() || test->Name().contains(*filter)) {
      selected_tests.push_back(test.get());
    }
  }
  // A filter that selects nothing is reported rather than run, so that a
  // mistyped one fails instead of quietly passing an empty suite.
  if (filter.has_value() && selected_tests.empty()) {
    ctx.Err() << "no test matches the filter '" << *filter << "'\n";
    return 1;
  }

  struct TestResult {
    std::string_view name;
    bool pass;
    std::chrono::microseconds elapsed_time;
    std::stringstream output;
  };

  std::vector<TestResult> test_results;
  test_results.reserve(selected_tests.size());
  for (const Test* test : selected_tests) {
    test_results.push_back(TestResult{.name = test->Name()});
  }

  const auto suite_start_time = std::chrono::system_clock::now();
  for (std::size_t i = 0; i < selected_tests.size(); ++i) {
    // Captures everything the test writes, including `ctx.Out()`, which
    // refers to this same `std::cout`.
    std::streambuf* original_cout_buf = std::cout.rdbuf();
    std::cout.rdbuf(test_results[i].output.rdbuf());

    const auto start_time = std::chrono::system_clock::now();
    const bool pass = selected_tests[i]->RunFull();
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
      ctx.Out() << "│ PASS " << test_result.name;

    } else {
      ctx.Out() << "│ FAIL " << test_result.name;
    }
    if (enable_timings) {
      ctx.Out() << " (" << test_result.elapsed_time << ")";
    }
    ctx.Out() << "\n";
  }

  const std::size_t passed_tests_count = std::ranges::count_if(
      test_results, [](const auto& test_result) { return test_result.pass; });
  const std::size_t failed_tests_count = std::ranges::count_if(
      test_results, [](const auto& test_result) { return !test_result.pass; });
  ctx.Out() << "└─► " << passed_tests_count << " out of " << test_results.size()
            << " tests PASS";
  if (enable_timings) {
    ctx.Out() << "(" << suite_elapsed_time << ")";
  }
  ctx.Out() << "\n";

  ctx.Out() << "\n";

  for (const auto& test_result : test_results) {
    if (test_result.pass) continue;

    ctx.Out() << "== " << test_result.name << " ==\n";
    ctx.Out() << test_result.output.str() << "\n";
  }

  return static_cast<int>(failed_tests_count);
}

}  // namespace

bool Test::RunFull() {
  std::error_code get_temp_dir_base_error_code;
  std::filesystem::path temp_dir_base =
      std::filesystem::temp_directory_path(get_temp_dir_base_error_code);
  if (get_temp_dir_base_error_code) {
    std::cout << "Fail: couldn't get base temp directory for test run: "
              << get_temp_dir_base_error_code.message() << "\n";
    return false;
  }

  std::string temp_dir = temp_dir_base / (std::string(Name()) + "-XXXXXX");
  if (mkdtemp(temp_dir.data()) == nullptr) {
    std::cout << "Fail: couldn't create temp directory for test run\n";
    return false;
  }
  temp_dir_ = temp_dir;

  failed_ = false;
  Run();

  std::error_code remove_temp_dir_error_code;
  std::filesystem::remove_all(temp_dir_, remove_temp_dir_error_code);
  if (remove_temp_dir_error_code) {
    std::cout << "Fail: couldn't remove temp directory for test run: "
              << remove_temp_dir_error_code.message() << "\n";
    return false;
  }

  return !failed_;
}

int AddTest(std::unique_ptr<Test> test) {
  kRegisteredTests.push_back(std::move(test));
  return 1;
}

// Runs the tests in this binary, parsing `args` for the flags below.
//
// The suite is the only thing this binary does, so it is registered as a
// command named after the binary itself rather than as a subcommand: flags are
// then parsed wherever they appear, instead of only in the first position.
int RunAllTests(std::vector<std::string_view> args) {
  args[0] = "tests";
  return RunCommand({{
                        .name = args[0],
                        .help = "Runs the tests in this binary.",
                        .flags = {{
                                      .name = "disable_timings",
                                      .help = "Omits timings from the summary.",
                                  },
                                  {
                                      .name = "filter",
                                      .help = "Runs only the tests whose name "
                                              "contains this substring.",
                                  }},
                        .handler = HandleRunTestsCommand,
                    }},
                    StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::RunAllTests({argv, argv + argc});
}
