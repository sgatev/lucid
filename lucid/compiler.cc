#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cpp_gen.h"

namespace lucid {

int Main() {
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
                                      allocate(IntLit{.value = 21}),
                                  },
                          }),
                      }),
                  },
          },
      .result_type = "int",
  });

  std::FILE* cpp_source_file = std::tmpfile();
  std::fputs(GenerateCppSource(arena, arena.get(id_func_ref)).c_str(),
             cpp_source_file);
  std::fputs(GenerateCppSource(arena, arena.get(main_func_ref)).c_str(),
             cpp_source_file);
  std::fseek(cpp_source_file, 0, SEEK_SET);

  dup2(fileno(cpp_source_file), 0);
  std::system("cc -o main -x c++ -");

  return 0;
}

}  // namespace lucid

int main() { return lucid::Main(); }
