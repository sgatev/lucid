#include "lucid/am_gen.h"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

std::size_t ComputeMaxStackSize(const Arena<Stmt>& arena,
                                const ControlFlowGraph& graph) {
  std::size_t stack_size = 0;
  stack_size += graph.func_params.size() * 4;
  for (const auto& block : graph.blocks()) {
    for (const auto& stmt_ref : block.statements) {
      const auto& stmt = arena.get(stmt_ref);
      if (auto* var_decl = std::get_if<VarDeclStmt>(&stmt)) {
        stack_size += 4;
      }
    }
  }
  return stack_size;
}

class AbstractMachineInstructionGenerator {
 public:
  AbstractMachineInstructionGenerator(const Arena<Stmt>& arena,
                                      const ControlFlowGraph& graph,
                                      AbstractMachineState& state)
      : arena_(arena),
        graph_(graph),
        state_(state),
        stack_size_(ComputeMaxStackSize(arena_, graph_)) {}

  void Generate() {
    state_.out_reg.clear();
    state_.out_reg.reserve(arena_.size());

    std::size_t instructions_count = 0;
    for (const auto& block : graph_.blocks()) {
      instructions_count += block.statements.size();
    }
    state_.instructions.clear();
    state_.instructions.reserve(instructions_count * 2);

    state_.instructions.push_back(PushStack{.size = stack_size_});
    for (int i = 0; i < graph_.func_params.size(); ++i) {
      const auto& param = graph_.func_params[i];
      if (param.type == "Int32") {
        state_.instructions.push_back(StoreStack32{
            .offset = stack_offset_,
            .src_reg = i + 1,
        });
        var_stack_[graph_.func_params[i].name] = stack_offset_;
        stack_offset_ += 4;
      } else if (param.type == "Int64") {
        state_.instructions.push_back(StoreStack64{
            .offset = stack_offset_,
            .src_reg = i + 1,
        });
        var_stack_[graph_.func_params[i].name] = stack_offset_;
        stack_offset_ += 8;
      }
    }
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
    int i = 0;
    for (auto next_block : block.next) {
      if (next_block == graph_.last) continue;

      state_.instructions.push_back(Label{
          .id = next_label_id_ + i,
      });
      ++i;

      Process(graph_.get(next_block));
    }
    next_label_id_ += i;
  }

  void Process(StmtRef ref, const Stmt& stmt) {
    std::visit([this, ref](auto&& stmt) { Process(ref, stmt); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt) {
    const auto& value = std::get<Expr>(DerefStmt(stmt.value));
    auto type = std::visit([](auto& expr) { return expr.type; }, value);

    if (type == "Int32") {
      state_.instructions.push_back(MoveReg32{
          .src_reg = state_.out_reg[stmt.value],
          .dst_reg = 0,
      });
    } else if (type == "Int64") {
      state_.instructions.push_back(MoveReg64{
          .src_reg = state_.out_reg[stmt.value],
          .dst_reg = 0,
      });
    }

    state_.instructions.push_back(PopStack{.size = stack_size_});
    state_.instructions.push_back(Return{});
  }

  void Process(StmtRef ref, const Expr& expr) {
    std::visit([this, ref](auto&& expr) { ProcessExpr(ref, expr); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr) {
    RegId reg = next_reg_++;
    if (expr.type == "Int64") {
      state_.instructions.push_back(SetReg64{
          .src_val = expr.value,
          .dst_reg = reg,
      });
    } else {
      // TODO: Make this conditional on the type once func param types are
      // available.
      state_.instructions.push_back(SetReg32{
          .src_val = expr.value,
          .dst_reg = reg,
      });
    }
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

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {
    if (stmt.type == "Int32") {
      state_.instructions.push_back(StoreStack32{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init],
      });
      var_stack_[stmt.name] = stack_offset_;
      stack_offset_ += 4;
    } else if (stmt.type == "Int64") {
      state_.instructions.push_back(StoreStack64{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init],
      });
      var_stack_[stmt.name] = stack_offset_;
      stack_offset_ += 8;
    }
  }

  void ProcessExpr(ExprRef ref, const IdentExpr& expr) {
    RegId reg = next_reg_++;
    if (expr.type == "Int32") {
      state_.instructions.push_back(LoadStack32{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    } else if (expr.type == "Int64") {
      state_.instructions.push_back(LoadStack64{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr) {
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        if (expr.type == "Int32") {
          state_.instructions.push_back(AddReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr.type == "Int64") {
          state_.instructions.push_back(AddReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Sub:
        if (expr.type == "Int32") {
          state_.instructions.push_back(SubReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr.type == "Int64") {
          state_.instructions.push_back(SubReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Mul:
        if (expr.type == "Int32") {
          state_.instructions.push_back(MulReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr.type == "Int64") {
          state_.instructions.push_back(MulReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Div:
        if (expr.type == "Int32") {
          state_.instructions.push_back(DivReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr.type == "Int64") {
          state_.instructions.push_back(DivReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Gt:
        if (expr.type == "Int64") {
          state_.instructions.push_back(GtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.instructions.push_back(GtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Lt:
        if (expr.type == "Int64") {
          state_.instructions.push_back(LtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.instructions.push_back(LtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Eq:
        if (expr.type == "Int64") {
          state_.instructions.push_back(EqReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.instructions.push_back(EqReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
    }
    state_.out_reg[ref] = reg;
  }

  const Stmt& DerefStmt(StmtRef ref) const { return arena_.get(ref); }

  const Arena<Stmt>& arena_;
  const ControlFlowGraph& graph_;
  AbstractMachineState& state_;
  std::size_t stack_size_;
  RegId next_reg_ = 1;
  std::size_t next_label_id_ = 1;
  std::size_t stack_offset_ = 0;
  std::map<std::string_view, std::size_t> var_stack_;
};

}  // namespace

void GenerateAbstractMachineInstructions(const Arena<Stmt>& arena,
                                         const ControlFlowGraph& graph,
                                         AbstractMachineState& state) {
  AbstractMachineInstructionGenerator(arena, graph, state).Generate();
}

}  // namespace lucid
