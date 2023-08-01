#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#include "lucid/arena.h"
#include "lucid/arm_assembly_gen.h"
#include "lucid/ast.h"

namespace lucid {

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
      std::string("as -arch arm64 -o ") + std::string(input) + ".o -- ";
  std::system(as_cmd.data());

  const std::string ld_cmd = std::string("ld -o ") + std::string(input) + " " +
                             std::string(input) +
                             ".o -lSystem -syslibroot `xcrun -sdk macosx "
                             "--show-sdk-path` -e _start -arch arm64";
  std::system(ld_cmd.data());

  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Main(argv[1]); }
