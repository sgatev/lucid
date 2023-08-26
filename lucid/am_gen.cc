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
                                      const ControlFlowGraph& graph,
                                      AbstractMachineState& state)
      : arena_(arena), graph_(graph), state_(state) {}

  void Generate() {
    state_.out_reg.clear();
    state_.out_reg.reserve(arena_.size());

    std::size_t instructions_count = 0;
    for (const auto& block : graph_.blocks()) {
      instructions_count += block.statements.size();
    }
    state_.instructions.clear();
    state_.instructions.reserve(instructions_count * 2);

    Process(graph_.get(graph_.first));
  }

 private:
  void Process(const ControlFlowGraph::Block& block) {
    for (const auto& stmt_ref : block.statements) {
      Process(stmt_ref, DerefStmt(stmt_ref));
    }

    if (block.branch_cond != ControlFlowGraph::kNullBlockRef) {
      state_.instructions.push_back(CondJump{
          .cond_reg = state_.out_reg[block.branch_cond],
          .then_label = next_label_id_,
          .else_label = next_label_id_ + 1,
      });
    }
    for (auto next_block : block.next) {
      if (next_block == graph_.last) continue;

      state_.instructions.push_back(Label{
          .id = next_label_id_++,
      });

      Process(graph_.get(next_block));
    }
  }

  void Process(StmtRef ref, const Stmt& stmt) {
    std::visit([this, ref](auto&& stmt) { Process(ref, stmt); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt) {
    state_.instructions.push_back(MoveReg32{
        .src_reg = state_.out_reg[stmt.value],
        .dst_reg = 0,
    });
    state_.instructions.push_back(Return{});
  }

  void Process(StmtRef ref, const Expr& expr) {
    std::visit([this, ref](auto&& expr) { ProcessExpr(ref, expr); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr) {
    RegId reg = next_reg_++;
    state_.instructions.push_back(SetReg32{
        .src_val = expr.value,
        .dst_reg = reg,
    });
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr) {
    RegId reg = next_reg_++;
    state_.instructions.push_back(SetReg32{
        .src_val = expr.value == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr) {
    state_.instructions.push_back(Jump{
        .label = expr.func_name,
    });
    state_.out_reg[ref] = 0;
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {}

  void ProcessExpr(ExprRef ref, const IdentExpr& expr) {
    state_.out_reg[ref] = 1;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr) {
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        state_.instructions.push_back(AddReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
      case BinaryOp::Sub:
        state_.instructions.push_back(SubReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
      case BinaryOp::Mul:
        state_.instructions.push_back(MulReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
      case BinaryOp::Div:
        state_.instructions.push_back(DivReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
      case BinaryOp::Gt:
        state_.instructions.push_back(GtReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
      case BinaryOp::Lt:
        state_.instructions.push_back(LtReg32{
            .res_reg = reg,
            .lhs_reg = state_.out_reg[expr.lhs],
            .rhs_reg = state_.out_reg[expr.rhs],
        });
        break;
    }
    state_.out_reg[ref] = reg;
  }

  const Stmt& DerefStmt(StmtRef ref) const { return arena_.get(ref); }

  const Arena<Stmt>& arena_;
  const ControlFlowGraph& graph_;
  AbstractMachineState& state_;
  RegId next_reg_ = 1;
  std::size_t next_label_id_ = 1;
};

}  // namespace

void GenerateAbstractMachineInstructions(const Arena<Stmt>& arena,
                                         const ControlFlowGraph& graph,
                                         AbstractMachineState& state) {
  AbstractMachineInstructionGenerator(arena, graph, state).Generate();
}

}  // namespace lucid
