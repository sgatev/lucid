#include "lucid/arm_assembly_gen.h"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

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
    Append(".global _start\n");
    Append(".align 2\n");
    Append("_start:\n");
    Indent();
    AppendIndent();
    Append("stp X29, X30, [sp, #-16]!\n");
    AppendIndent();
    Append("BL main\n");
    AppendIndent();
    Append("ldp X29, X30, [sp], #16\n");
    AppendIndent();
    Append("mov X16, #1\n");
    AppendIndent();
    Append("svc #0x80\n");
    UnIndent();
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
    for (const auto& stmt_ref : block.statements)
      Process(stmt_ref, DerefStmt(stmt_ref));
  }

  void Process(StmtRef stmt_ref, const Stmt& stmt) {
    std::visit([this, stmt_ref](auto&& stmt) { Process(stmt_ref, stmt); },
               stmt);
  }

  void Process(StmtRef stmt_ref, const ReturnStmt& stmt) {
    AppendIndent();
    Append("mov X0, ");
    Append(out_reg_[stmt.value]);
    Append("\n");
    AppendIndent();
    Append("RET\n");
  }

  void Process(StmtRef stmt_ref, const Expr& expr) {
    std::visit([this, stmt_ref](auto&& expr) { ProcessExpr(stmt_ref, expr); },
               expr);
  }

  void ProcessExpr(ExprRef expr_ref, const IntLitExpr& expr) {
    AppendIndent();
    Append("mov X1, #");
    Append(expr.value);
    Append("\n");

    out_reg_[expr_ref] = "X1";
  }

  void ProcessExpr(ExprRef expr_ref, const FuncCallExpr& expr) {
    AppendIndent();
    Append("stp X29, X30, [sp, #-16]!\n");
    AppendIndent();
    Append("BL ");
    Append(expr.func_name);
    Append("\n");
    AppendIndent();
    Append("ldp X29, X30, [sp], #16\n");

    out_reg_[expr_ref] = "X0";
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {}

  void ProcessExpr(ExprRef expr_ref, const IdentExpr& expr) {
    out_reg_[expr_ref] = "X1";
  }

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
  std::map<StmtRef, std::string> out_reg_;
};

}  // namespace

std::string GenerateArmAssemblySource(const Arena<Stmt>& arena,
                                      const std::vector<FuncDefStmt>& funcs) {
  ArmAssemblySourceGenerator gen(arena);
  for (const auto& func : funcs) gen.Process(func);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
