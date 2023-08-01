#include "lucid/arm_assembly_gen.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/string_builder.h"

namespace lucid {
namespace {

// Generates 64-bit ARM assembly source code.
class ArmAssemblySourceGenerator {
 public:
  explicit ArmAssemblySourceGenerator(const Arena<Stmt>& arena)
      : arena_(arena) {
    Append(".global main\n");
    Append(".align 2\n");
  }

  // Adds source code for `func` to the generated source.
  void Process(const FuncDefStmt& func) {
    Append(func.name);
    Append(":\n");
    Indent();
    auto control_flow_graph = BuildControlFlowGraph(arena_, func);
    Process(control_flow_graph.get(control_flow_graph.first));
    UnIndent();
  }

  // Extracts and returns the source code produced by this generator.
  std::string ConsumeGeneratedSource() && {
    return std::move(output_builder_).Build();
  }

 private:
  void Process(const ControlFlowGraph::Block& block) {
    for (const auto& stmt_ref : block.statements) Process(DerefStmt(stmt_ref));
  }

  void Process(const Stmt& stmt) {
    std::visit([this](auto&& stmt) { Process(stmt); }, stmt);
  }

  void Process(const CompoundStmt& stmt) {
    // Must not happen.
  }

  void Process(const ReturnStmt& stmt) {
    AppendIndent();
    Append("mov X0, X1\n");
    AppendIndent();
    Append("mov X16, #1\n");
    AppendIndent();
    Append("svc #0x80\n");
  }

  void Process(const Expr& expr) {
    std::visit([this](auto&& expr) { ProcessExpr(expr); }, expr);
  }

  void ProcessExpr(const IntLitExpr& expr) {
    AppendIndent();
    Append("mov X1, #");
    Append(expr.value);
    Append("\n");
  }

  void ProcessExpr(const FuncCallExpr& expr) {}

  void Process(const VarDeclStmt& stmt) {}

  void ProcessExpr(const IdentExpr& expr) {}

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

std::string GenerateArmAssemblySource(const Arena<Stmt>& arena,
                                      const FuncDefStmt& func) {
  ArmAssemblySourceGenerator gen(arena);
  gen.Process(func);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
