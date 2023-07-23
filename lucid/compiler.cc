#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cpp_gen.h"

namespace lucid {

void GenerateEmptyMain(std::FILE* cpp_source_file) {
  Arena<Stmt> arena;
  auto allocate = [&arena](auto&& stmt) { return arena.add(stmt); };

  auto main_func_ref = allocate(FuncDefStmt{
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
  });

  std::fputs(GenerateCppSource(arena, arena.get(main_func_ref)).c_str(),
             cpp_source_file);
}

void GenerateFuncCall(std::FILE* cpp_source_file) {
  Arena<Stmt> arena;
  auto allocate = [&arena](auto&& stmt) { return arena.add(stmt); };

  auto id_func_ref = allocate(FuncDefStmt{
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
  });

  auto main_func_ref = allocate(FuncDefStmt{
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
  });

  std::fputs(GenerateCppSource(arena, arena.get(id_func_ref)).c_str(),
             cpp_source_file);
  std::fputs(GenerateCppSource(arena, arena.get(main_func_ref)).c_str(),
             cpp_source_file);
}

int Main(std::string_view input) {
  std::FILE* cpp_source_file = std::tmpfile();
  if (input == "empty_main") {
    GenerateEmptyMain(cpp_source_file);
  } else if (input == "func_call") {
    GenerateFuncCall(cpp_source_file);
  }
  std::fseek(cpp_source_file, 0, SEEK_SET);

  dup2(fileno(cpp_source_file), 0);
  const std::string command =
      std::string("cc -o ") + std::string(input) + " -x c++ -";
  std::system(command.data());

  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Main(argv[1]); }
