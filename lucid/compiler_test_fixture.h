#pragma once

#include <sys/wait.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/cli.h"
#include "lucid/file.h"

namespace lucid {

// Represents the result of a command.
struct CommandResult {
  // Return code of the command.
  int return_code;

  // String printed on stdout by the command.
  std::string out;

  // String printed on stderr by the command.
  std::string err;
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& stream,
                                          const CommandResult& res) {
  return stream << "CommandResult{.return_code=" << res.return_code
                << ", .out=\"" << res.out << "\" "
                << ", .err=\"" << res.err << "\"}";
}

// Matches the return code of a command.
MATCHER_P(ReturnsCode, matcher, "") {
  return ExplainMatchResult(matcher, arg.return_code, result_listener);
}

// Matches the string printed on stdout by a command.
MATCHER_P(Prints, matcher, "") {
  return ExplainMatchResult(matcher, arg.out, result_listener);
}

// Matches the string printed on stderr by a command.
MATCHER_P(PrintsError, matcher, "") {
  return ExplainMatchResult(matcher, arg.err, result_listener);
}

// A fixture that can be used to test both the compiler and the compiled binary,
class CompilerTest : public testing::Test {
 protected:
  // Creates a file with name `src_file_name` and sets its content  to `src`.
  bool CreateFile(std::string_view src_file_name, std::string_view src) {
    const std::string src_path = runtime_path_ / src_file_name;
    std::ofstream src_stream(src_path);
    src_stream << src;
    return true;
  }

  // Runs the compiler binary with the given `args`.
  CommandResult RunCompiler(std::initializer_list<std::string_view> args) {
    std::string cmd = FullPath("lucid/compiler");
    for (auto arg : args) cmd += " " + std::string(arg);
    return ExecCommand(cmd);
  }

  // Runs a binary with the given `binary_name`.
  CommandResult Run(std::string_view binary_name) {
    return ExecCommand(FullPath(binary_name));
  }

  // Returns the full path of the file with the given `file_name`.
  std::string FullPath(std::string_view file_name) {
    return runtime_path_ / file_name;
  }

 private:
  CommandResult ExecCommand(std::string_view command) {
    const std::string out_path = runtime_path_ / "stdout";
    const std::string err_path = runtime_path_ / "stderr";
    std::string c = std::string(command) + " > " + out_path + " 2> " + err_path;
    const int result = std::system(c.c_str());
    const int return_code = WEXITSTATUS(result);
    return {
        .return_code = return_code,
        .out = ReadFile(out_path).value(),
        .err = ReadFile(err_path).value(),
    };
  }

  const std::filesystem::path runtime_path_ = testing::SrcDir() + "__main__";
};

}  // namespace lucid
