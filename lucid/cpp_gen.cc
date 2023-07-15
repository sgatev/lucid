#include "lucid/cpp_gen.h"

#include <string>
#include <variant>

#include "lucid/ast.h"

namespace lucid {
namespace {

// Generates C++ source code.
class CppSourceGenerator {
 public:
  // Adds source code for `stmt` to the generated source.
  void Process(const Stmt& stmt) {
    std::visit([this](auto&& stmt) { Process(stmt); }, stmt);
  }

  // Extracts and returns the source code produced by this generator.
  std::string ConsumeGeneratedSource() && { return std::move(source_); }

 private:
  void Process(const FuncDefStmt& stmt) {
    source_.append("void");
    source_.append(" ");
    source_.append(stmt.name);
    source_.append("() {}");
  }

  std::string source_;
};

}  // namespace

std::string GenerateCppSource(const Stmt& stmt) {
  CppSourceGenerator gen;
  gen.Process(stmt);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
