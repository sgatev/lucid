#include "lucid/cpp_gen.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/string_builder.h"

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
    Append(";\n");
  }

  // Extracts and returns the source code produced by this generator.
  std::string ConsumeGeneratedSource() && {
    return std::move(output_builder_).Build();
  }

 private:
  void Process(const FuncDefStmt& stmt) {
    Append(stmt.result_type);
    Append(" ");
    Append(stmt.name);
    Append("(");
    bool notFirst = false;
    for (const auto& param : stmt.parameters) {
      if (notFirst) Append(", ");

      Append(param.type);
      Append(" ");
      Append(param.name);

      notFirst = true;
    }
    Append(") ");
    Process(stmt.body);
  }

  void Process(const CompoundStmt& stmt) {
    Append("{\n");
    Indent();
    for (const auto& stmt_ref : stmt.statements) {
      Process(DerefStmt(stmt_ref));
    }
    UnIndent();
    Append("}");
  }

  void Process(const ReturnStmt& stmt) {
    Append("return");
    Append(" ");
    Process(DerefExpr(stmt.value));
  }

  void Process(const Expr& expr) {
    std::visit([this](auto&& expr) { Process(expr); }, expr);
  }

  void Process(const IntLitExpr& expr) { Append(expr.value); }

  void Process(const FuncCallExpr& expr) {
    Append(expr.func_name);
    Append("(");
    bool notFirst = false;
    for (const auto& arg : expr.arguments) {
      if (notFirst) Append(", ");

      Process(DerefExpr(arg));

      notFirst = true;
    }
    Append(")");
  }

  void Process(const VarDeclStmt& stmt) {
    Append(stmt.type);
    Append(" ");
    Append(stmt.name);
    Append(" = ");
    Process(DerefExpr(stmt.init));
  }

  void Process(const IdentExpr& expr) { Append(expr.name); }

  void Indent() { indent_ += 2; }

  void UnIndent() { indent_ -= 2; }

  void AppendIndent() {
    for (int i = 0; i < indent_; ++i) Append(" ");
  }

  const Stmt& DerefStmt(StmtRef ref) { return arena_.get(ref); }
  const Expr& DerefExpr(ExprRef ref) { return std::get<Expr>(DerefStmt(ref)); }

  void Append(std::string_view s) { output_builder_.Append(s); }

  const Arena<Stmt>& arena_;
  StringBuilder output_builder_;
  std::size_t indent_ = 0;
};

}  // namespace

std::string GenerateCppSource(const Arena<Stmt>& arena, const Stmt& stmt) {
  CppSourceGenerator gen(arena);
  gen.Process(stmt);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
