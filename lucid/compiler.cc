#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "lucid/arena.h"
#include "lucid/arm_assembly_gen.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

// Format `args` according to the format string `fmt`.
//
// From https://stackoverflow.com/a/26221725.
template <typename... Args>
std::string string_format(const std::string& fmt, Args... args) {
  int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
  if (size < 0) throw std::runtime_error("Error during formatting.");

  std::string result;
  result.resize(size);
  std::snprintf(result.data(), size, fmt.c_str(), args...);
  return result;
}

}  // namespace

void GenerateEmptyMain(std::FILE* out) {
  Arena<Stmt> arena;
  auto allocate = [&arena](auto&& stmt) { return arena.add(stmt); };

  auto main_func = FuncDefStmt{
      .name = "main",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      allocate(ReturnStmt{
                          .value = allocate(IntLitExpr{.value = "0"}),
                      }),
                  },
          },
  };

  std::fputs(GenerateArmAssemblySource(arena, {main_func}).c_str(), out);
}

void GenerateFuncCall(std::FILE* out) {
  Arena<Stmt> arena;
  auto allocate = [&arena](auto&& stmt) { return arena.add(stmt); };

  auto id_func = FuncDefStmt{
      .name = "id",
      .parameters =
          {
              FuncParam{
                  .name = "x",
                  .type = "int",
              },
          },
      .body =
          {
              .statements =
                  {
                      allocate(ReturnStmt{
                          .value = allocate(IdentExpr{.name = "x"}),
                      }),
                  },
          },
      .result_type = "int",
  };

  auto main_func = FuncDefStmt{
      .name = "main",
      .body =
          {
              .statements =
                  {
                      allocate(ReturnStmt{
                          .value = allocate(FuncCallExpr{
                              .func_name = "id",
                              .arguments =
                                  {
                                      allocate(IntLitExpr{.value = "21"}),
                                  },
                          }),
                      }),
                  },
          },
      .result_type = "int",
  };

  std::fputs(GenerateArmAssemblySource(arena, {id_func, main_func}).c_str(),
             out);
}

int Main(std::string_view input) {
  std::FILE* out = std::tmpfile();
  if (input == "empty_main") {
    GenerateEmptyMain(out);
  } else if (input == "func_call") {
    GenerateFuncCall(out);
  }
  std::fseek(out, 0, SEEK_SET);

  dup2(fileno(out), 0);
  const std::string as_cmd =
      string_format("as -arch arm64 -o %s.o -- ", input.data());
  std::system(as_cmd.data());

  const std::string ld_cmd = string_format(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      input.data(), input.data());
  std::system(ld_cmd.data());

  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Main(argv[1]); }
