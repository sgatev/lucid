#include "lucid/cpp_gen.h"

#include <cstddef>
#include <string>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

// Generates C++ source code.
class CppSourceGenerator {
 public:
  explicit CppSourceGenerator(const Arena<Stmt>& arena) : arena_(arena) {}

  // Adds source code for `stmt` to the generated source.
  void Process(const Stmt& stmt) {
    AppendIndent();
    std::visit([this](auto&& stmt) { Process(stmt); }, stmt);
    source_.append(";\n");
  }

  // Extracts and returns the source code produced by this generator.
  std::string ConsumeGeneratedSource() && { return std::move(source_); }

 private:
  void Process(const FuncDefStmt& stmt) {
    source_.append(stmt.result_type);
    source_.append(" ");
    source_.append(stmt.name);
    source_.append("(");
    bool notFirst = false;
    for (const auto& param : stmt.parameters) {
      if (notFirst) source_.append(", ");

      source_.append(param.type);
      source_.append(" ");
      source_.append(param.name);

      notFirst = true;
    }
    source_.append(") ");
    Process(stmt.body);
  }

  void Process(const CompoundStmt& stmt) {
    source_.append("{\n");
    Indent();
    for (const auto& stmt_ref : stmt.statements) {
      Process(DerefStmt(stmt_ref));
    }
    UnIndent();
    source_.append("}");
  }

  void Process(const ReturnStmt& stmt) {
    source_.append("return");
    source_.append(" ");
    Process(DerefExpr(stmt.value));
  }

  void Process(const Expr& expr) {
    std::visit([this](auto&& expr) { Process(expr); }, expr);
  }

  void Process(const IntLit& lit) { source_.append(std::to_string(lit.value)); }

  void Process(const FuncCallExpr& expr) {
    source_.append(expr.func_name);
    source_.append("()");
  }

  void Process(const VarDeclStmt& stmt) {
    source_.append(stmt.type);
    source_.append(" ");
    source_.append(stmt.name);
    source_.append(" = ");
    Process(DerefExpr(stmt.init));
  }

  void Process(const IdentExpr& expr) { source_.append(expr.name); }

  void Indent() { indent_ += 2; }
  void UnIndent() { indent_ -= 2; }
  void AppendIndent() { source_.append(std::string(indent_, ' ')); }

  const Stmt& DerefStmt(StmtRef ref) { return arena_.get(ref); }
  const Expr& DerefExpr(ExprRef ref) { return std::get<Expr>(DerefStmt(ref)); }

  const Arena<Stmt>& arena_;
  std::string source_;
  std::size_t indent_ = 0;
};

}  // namespace

std::string GenerateCppSource(const Arena<Stmt>& arena, const Stmt& stmt) {
  CppSourceGenerator gen(arena);
  gen.Process(stmt);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
