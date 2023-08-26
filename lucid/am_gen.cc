#include "lucid/am_gen.h"

#include <cstddef>
#include <string>
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
      : arena_(arena), graph_(graph), out_reg_(arena_.size()) {}

  std::vector<Instruction> Generate() && {
    std::size_t instructions_count = 0;
    for (const auto& block : graph_.blocks()) {
      instructions_count += block.statements.size();
    }
    instructions_.reserve(instructions_count * 2);

    Process(graph_.get(graph_.first));
    return std::move(instructions_);
  }

 private:
  void Process(const ControlFlowGraph::Block& block) {
    for (const auto& stmt_ref : block.statements) {
      Process(stmt_ref, DerefStmt(stmt_ref));
    }

    if (block.branch_cond != ControlFlowGraph::kNullBlockRef) {
      instructions_.push_back(CondJump{
          .cond_reg = out_reg_[block.branch_cond],
          .then_label = next_label_id_,
          .else_label = next_label_id_ + 1,
      });
    }
    for (auto next_block : block.next) {
      if (next_block == graph_.last) continue;

      instructions_.push_back(Label{
          .id = next_label_id_++,
      });

      Process(graph_.get(next_block));
    }
  }

  void Process(StmtRef ref, const Stmt& stmt) {
    std::visit([this, ref](auto&& stmt) { Process(ref, stmt); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt) {
    instructions_.push_back(MoveReg32{
        .src_reg = out_reg_[stmt.value],
        .dst_reg = 0,
    });
    instructions_.push_back(Return{});
  }

  void Process(StmtRef ref, const Expr& expr) {
    std::visit([this, ref](auto&& expr) { ProcessExpr(ref, expr); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr) {
    RegId reg = next_reg_++;
    instructions_.push_back(SetReg32{
        .src_val = expr.value,
        .dst_reg = reg,
    });
    out_reg_[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr) {
    RegId reg = next_reg_++;
    instructions_.push_back(SetReg32{
        .src_val = expr.value == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    out_reg_[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr) {
    instructions_.push_back(Jump{
        .label = expr.func_name,
    });
    out_reg_[ref] = 0;
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {}

  void ProcessExpr(ExprRef ref, const IdentExpr& expr) { out_reg_[ref] = 1; }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr) {
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        instructions_.push_back(AddReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Sub:
        instructions_.push_back(SubReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Mul:
        instructions_.push_back(MulReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Div:
        instructions_.push_back(DivReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Gt:
        instructions_.push_back(GtReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
      case BinaryOp::Lt:
        instructions_.push_back(LtReg32{
            .res_reg = reg,
            .lhs_reg = out_reg_[expr.lhs],
            .rhs_reg = out_reg_[expr.rhs],
        });
        break;
    }
    out_reg_[ref] = reg;
  }

  const Stmt& DerefStmt(StmtRef ref) const { return arena_.get(ref); }

  const Arena<Stmt>& arena_;
  const ControlFlowGraph& graph_;
  std::vector<RegId> out_reg_;
  std::vector<Instruction> instructions_;
  RegId next_reg_ = 1;
  std::size_t next_label_id_ = 1;
};

}  // namespace

std::vector<Instruction> GenerateAbstractMachineInstructions(
    const Arena<Stmt>& arena, const ControlFlowGraph& graph) {
  return AbstractMachineInstructionGenerator(arena, graph).Generate();
}

}  // namespace lucid
