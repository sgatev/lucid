#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/arm64_gen.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/writer.h"

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

std::vector<FuncDefStmt> GenerateEmptyMain(Arena<Stmt>& arena) {
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

  return {main_func};
}

std::vector<FuncDefStmt> GenerateFuncCall(Arena<Stmt>& arena) {
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

  return {id_func, main_func};
}

std::vector<FuncDefStmt> GenerateAddInts(Arena<Stmt>& arena) {
  auto allocate = [&arena](auto&& stmt) { return arena.add(stmt); };

  auto main_func = FuncDefStmt{
      .name = "main",
      .result_type = "int",
      .body =
          {
              .statements =
                  {
                      allocate(ReturnStmt{
                          .value = allocate(AddExpr{
                              .lhs = allocate(IntLitExpr{.value = "2"}),
                              .rhs = allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  return {main_func};
}

int Main(std::string_view input) {
  // Load Lucis sources.
  Arena<Stmt> arena;
  std::vector<FuncDefStmt> funcs;
  if (input == "empty_main") {
    funcs = GenerateEmptyMain(arena);
  } else if (input == "func_call") {
    funcs = GenerateFuncCall(arena);
  } else if (input == "add_ints") {
    funcs = GenerateAddInts(arena);
  }

  // Generate 64-bit ARM assembly.
  std::FILE* out = std::tmpfile();
  auto writer = FileWriter(out);
  GenerateArmStartSource(writer);
  for (const auto& func : funcs) {
    auto graph = BuildControlFlowGraph(arena, func);
    auto instructions = GenerateAbstractMachineInstructions(arena, graph);
    GenerateArmAssemblySource(func.name, instructions, writer);
  }
  std::fseek(out, 0, SEEK_SET);

  // Translate assembly into object code.
  dup2(fileno(out), 0);
  const std::string as_cmd =
      string_format("as -arch arm64 -o %s.o -- ", input.data());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = string_format(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      input.data(), input.data());
  std::system(ld_cmd.data());

  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Main(argv[1]); }
