#include "lucid/am_gen.h"

#include <map>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

class AbstractMachineInstructionGenerator {
 public:
  AbstractMachineInstructionGenerator(const Arena<Stmt>& arena,
                                      const ControlFlowGraph& graph)
      : arena_(arena), graph_(graph) {}

  std::vector<Instruction> Generate() && {
    Process(graph_.get(graph_.first));
    return std::move(instructions_);
  }

 private:
  void Process(const ControlFlowGraph::Block& block) {
    for (const auto& stmt_ref : block.statements) {
      Process(stmt_ref, DerefStmt(stmt_ref));
    }
  }

  void Process(StmtRef stmt_ref, const Stmt& stmt) {
    std::visit([this, stmt_ref](auto&& stmt) { Process(stmt_ref, stmt); },
               stmt);
  }

  void Process(StmtRef stmt_ref, const ReturnStmt& stmt) {
    instructions_.push_back(MoveReg32{
        .src_reg = out_reg_[stmt.value],
        .dst_reg = 0,
    });
    instructions_.push_back(Return{});
  }

  void Process(StmtRef stmt_ref, const Expr& expr) {
    std::visit([this, stmt_ref](auto&& expr) { ProcessExpr(stmt_ref, expr); },
               expr);
  }

  void ProcessExpr(ExprRef expr_ref, const IntLitExpr& expr) {
    RegId reg = next_reg_++;
    instructions_.push_back(SetReg32{
        .src_val = expr.value,
        .dst_reg = reg,
    });
    out_reg_[expr_ref] = reg;
  }

  void ProcessExpr(ExprRef expr_ref, const FuncCallExpr& expr) {
    instructions_.push_back(Jump{
        .label = expr.func_name,
    });
    out_reg_[expr_ref] = 0;
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {}

  void ProcessExpr(ExprRef expr_ref, const IdentExpr& expr) {
    out_reg_[expr_ref] = 1;
  }

  void ProcessExpr(ExprRef expr_ref, const BinaryOpExpr& expr) {
    switch (expr.op) {
      case BinaryOp::Add:
        instructions_.push_back(AddReg32{
            .res_reg = 3,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Sub:
        instructions_.push_back(SubReg32{
            .res_reg = 3,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Mul:
        instructions_.push_back(MulReg32{
            .res_reg = 3,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
    }
    out_reg_[expr_ref] = 3;
  }

  const Stmt& DerefStmt(StmtRef ref) { return arena_.get(ref); }

  const Arena<Stmt>& arena_;
  const ControlFlowGraph& graph_;
  std::map<StmtRef, RegId> out_reg_;
  std::vector<Instruction> instructions_;
  RegId next_reg_ = 1;
};

}  // namespace

std::vector<Instruction> GenerateAbstractMachineInstructions(
    const Arena<Stmt>& arena, const ControlFlowGraph& graph) {
  return AbstractMachineInstructionGenerator(arena, graph).Generate();
}

}  // namespace lucid
