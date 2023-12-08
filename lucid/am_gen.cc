#include "lucid/am_gen.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <variant>

#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

class AbstractMachineFunctionGenerator {
 public:
  AbstractMachineFunctionGenerator(const Arena<Stmt>& arena,
                                   const ControlFlowGraph& graph,
                                   AbstractMachineState& state)
      : arena_(arena), graph_(graph), state_(state) {
    for (const auto& param : graph.func_params) {
      const auto& param_type = std::get<BasicType>(DerefType(param.type));
      if (param_type.name == "Int32") {
        state_.func.stack_slots.push_back(4);
      } else if (param_type.name == "Int64") {
        state_.func.stack_slots.push_back(8);
      } else if (param_type.name == "Bool") {
        state_.func.stack_slots.push_back(1);
      }
    }
    for (const auto& block : graph.blocks()) {
      for (const auto& stmt_ref : block.statements) {
        const auto& stmt = arena.get(stmt_ref);
        if (auto* var_decl = std::get_if<VarDeclStmt>(&stmt)) {
          const auto& var_decl_type =
              std::get<BasicType>(DerefType(var_decl->type));
          if (var_decl_type.name == "Int32") {
            state_.func.stack_slots.push_back(4);
          } else if (var_decl_type.name == "Int64") {
            state_.func.stack_slots.push_back(8);
          } else if (var_decl_type.name == "Bool") {
            state_.func.stack_slots.push_back(1);
          }
        }
      }
    }
  }

  void Generate() {
    state_.out_reg.clear();
    state_.out_reg.reserve(arena_.size());

    state_.func.name = graph_.func_name;

    std::size_t instructions_count = 0;
    for (const auto& block : graph_.blocks()) {
      instructions_count += block.statements.size();
    }
    state_.func.instructions.clear();
    state_.func.instructions.reserve(instructions_count * 2);

    if (graph_.func_name == "printf") {
      state_.func.instructions.push_back(PushStack{});
      if (graph_.func_params.size() > 1) {
        state_.func.instructions.push_back(StoreStack64{
            .offset = 0,
            .src_reg = 2,
        });
      }
      state_.func.instructions.push_back(MoveReg64{
          .dst_reg = 0,
          .src_reg = 1,
      });
      state_.func.instructions.push_back(Jump{
          .label = "_printf",
      });
      state_.func.instructions.push_back(SetReg32{
          .dst_reg = 0,
          .src_val = "0",
      });
      state_.func.instructions.push_back(PopStack{});
      state_.func.instructions.push_back(Return{});
      return;
    }

    std::size_t push_pos, pop_pos;

    push_pos = state_.func.instructions.size();
    for (int i = 0; i < graph_.func_params.size(); ++i) {
      const auto& param = graph_.func_params[i];
      const auto& param_type = std::get<BasicType>(DerefType(param.type));
      if (param_type.name == "Int32") {
        state_.func.instructions.push_back(StoreStack32{
            .offset = stack_offset_,
            .src_reg = i + 1,
        });
        var_stack_[graph_.func_params[i].name] = stack_offset_;
        ++stack_offset_;
      } else if (param_type.name == "Int64") {
        state_.func.instructions.push_back(StoreStack64{
            .offset = stack_offset_,
            .src_reg = i + 1,
        });
        var_stack_[graph_.func_params[i].name] = stack_offset_;
        ++stack_offset_;
      }
    }

    for (const auto& block : graph_.blocks()) {
      state_.func.instructions.push_back(Label{
          .id = block.id,
      });

      if (block.id == graph_.get(graph_.last).id) {
        pop_pos = state_.func.instructions.size();
        state_.func.instructions.push_back(Return{});
      }

      for (const auto& stmt_ref : block.statements) {
        Process(stmt_ref, DerefStmt(stmt_ref));
      }

      if (block.branch_cond != ControlFlowGraph::kNullBlockRef) {
        state_.func.instructions.push_back(CondJump{
            .cond_reg = state_.out_reg[block.branch_cond],
            .then_label = graph_.get(block.next[0]).id,
            .else_label = graph_.get(block.next[1]).id,
        });
      } else if (block.next.size() == 1) {
        state_.func.instructions.push_back(UncondJump{
            .label = graph_.get(block.next[0]).id,
        });
      }
    }

    std::size_t offset = stack_offset_;

    state_.func.instructions.insert(state_.func.instructions.begin() + pop_pos,
                                    PopStack{});
    for (int i = next_reg_ - 1; i >= 1; --i) {
      state_.func.instructions.insert(
          state_.func.instructions.begin() + pop_pos,
          LoadStack64{
              .offset = offset + (i - 1),
              .dst_reg = i,
          });
      state_.func.stack_slots.push_back(8);
    }

    for (int i = next_reg_ - 1; i >= 1; --i) {
      state_.func.instructions.insert(
          state_.func.instructions.begin() + push_pos,
          StoreStack64{
              .offset = offset + (i - 1),
              .src_reg = i,
          });
    }
    state_.func.instructions.insert(state_.func.instructions.begin() + push_pos,
                                    PushStack{});
  }

 private:
  void Process(StmtRef ref, const Stmt& stmt) {
    std::visit([this, ref](auto&& stmt) { Process(ref, stmt); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt) {
    auto type = std::get<BasicType>(DerefType(GetType(DerefExpr(stmt.value))));
    if (type.name == "Int32") {
      state_.func.instructions.push_back(MoveReg32{
          .src_reg = state_.out_reg[stmt.value],
          .dst_reg = 0,
      });
    } else if (type.name == "Int64") {
      state_.func.instructions.push_back(MoveReg64{
          .src_reg = state_.out_reg[stmt.value],
          .dst_reg = 0,
      });
    } else if (type.name == "Bool") {
      state_.func.instructions.push_back(MoveReg32{
          .src_reg = state_.out_reg[stmt.value],
          .dst_reg = 0,
      });
    }
  }

  void Process(StmtRef ref, const Expr& expr) {
    std::visit([this, ref](auto&& expr) { ProcessExpr(ref, expr); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr) {
    auto expr_type = std::get<BasicType>(DerefType(expr.type));
    RegId reg = next_reg_++;
    if (expr_type.name == "Int64") {
      state_.func.instructions.push_back(SetReg64{
          .src_val = expr.value,
          .dst_reg = reg,
      });
    } else if (expr_type.name == "Int32") {
      state_.func.instructions.push_back(SetReg32{
          .src_val = expr.value,
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr) {
    RegId reg = next_reg_++;
    state_.func.instructions.push_back(SetReg32{
        .src_val = expr.value == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr) {
    auto string_id = reinterpret_cast<std::uintptr_t>(expr.value.data());
    state_.strings[string_id] = expr.value;

    RegId reg = next_reg_++;
    state_.func.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr) {
    RegId reg = next_reg_++;
    int i = 1;
    for (const auto arg : expr.arguments) {
      const auto& arg_expr = DerefExpr(arg);
      auto arg_type = std::get<BasicType>(DerefType(GetType(arg_expr)));
      if (arg_type.name == "Int32") {
        state_.func.instructions.push_back(MoveReg32{
            .src_reg = state_.out_reg[arg],
            .dst_reg = i++,
        });
      } else if (arg_type.name == "Int64") {
        state_.func.instructions.push_back(MoveReg64{
            .src_reg = state_.out_reg[arg],
            .dst_reg = i++,
        });
      } else if (arg_type.name == "String") {
        state_.func.instructions.push_back(MoveReg64{
            .src_reg = state_.out_reg[arg],
            .dst_reg = i++,
        });
      }
    }
    state_.func.instructions.push_back(Jump{
        .label = expr.func_name,
    });
    auto expr_type = std::get<BasicType>(DerefType(expr.type));
    if (expr_type.name == "Int32") {
      state_.func.instructions.push_back(MoveReg32{
          .src_reg = 0,
          .dst_reg = reg,
      });
    } else if (expr_type.name == "Int64") {
      state_.func.instructions.push_back(MoveReg64{
          .src_reg = 0,
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref] = reg;
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {
    auto stmt_type = std::get<BasicType>(DerefType(stmt.type));
    if (stmt_type.name == "Int32") {
      state_.func.instructions.push_back(StoreStack32{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init],
      });
      var_stack_[stmt.name] = stack_offset_;
      ++stack_offset_;
    } else if (stmt_type.name == "Int64") {
      state_.func.instructions.push_back(StoreStack64{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init],
      });
      var_stack_[stmt.name] = stack_offset_;
      ++stack_offset_;
    }
  }

  void Process(StmtRef stmt_ref, const VarAssignStmt& stmt) {
    auto expr_type =
        std::get<BasicType>(DerefType(GetType(DerefExpr(stmt.expr))));
    if (expr_type.name == "Int32") {
      state_.func.instructions.push_back(StoreStack32{
          .offset = var_stack_[stmt.name],
          .src_reg = state_.out_reg[stmt.expr],
      });
    } else if (expr_type.name == "Int64") {
      state_.func.instructions.push_back(StoreStack64{
          .offset = var_stack_[stmt.name],
          .src_reg = state_.out_reg[stmt.expr],
      });
    }
  }

  void Process(StmtRef stmt_ref, const BreakStmt& stmt) {}

  void ProcessExpr(ExprRef ref, const IdentExpr& expr) {
    auto expr_type = std::get<BasicType>(DerefType(expr.type));
    RegId reg = next_reg_++;
    if (expr_type.name == "Int32") {
      state_.func.instructions.push_back(LoadStack32{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    } else if (expr_type.name == "Int64") {
      state_.func.instructions.push_back(LoadStack64{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr) {
    auto expr_type =
        std::get<BasicType>(DerefType(GetType(DerefExpr(expr.lhs))));
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        if (expr_type.name == "Int32") {
          state_.func.instructions.push_back(AddReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(AddReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Sub:
        if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(SubReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.func.instructions.push_back(SubReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Mul:
        if (expr_type.name == "Int32") {
          state_.func.instructions.push_back(MulReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(MulReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Div:
        if (expr_type.name == "Int32") {
          state_.func.instructions.push_back(DivReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(DivReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Mod:
        if (expr_type.name == "Int32") {
          state_.func.instructions.push_back(ModReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(ModReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Gt:
        if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(GtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.func.instructions.push_back(GtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Lt:
        if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(LtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.func.instructions.push_back(LtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::Eq:
        if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(EqReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.func.instructions.push_back(EqReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
      case BinaryOp::NotEq:
        if (expr_type.name == "Int64") {
          state_.func.instructions.push_back(NotEqReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        } else {
          state_.func.instructions.push_back(NotEqReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs],
              .rhs_reg = state_.out_reg[expr.rhs],
          });
        }
        break;
    }
    state_.out_reg[ref] = reg;
  }

  void ProcessExpr(ExprRef ref, const Type& expr) {
    // TODO: How to generate code for types?
  }

  const Stmt& DerefStmt(StmtRef ref) const { return arena_.get(ref); }

  const Expr& DerefExpr(ExprRef ref) const {
    return std::get<Expr>(DerefStmt(ref));
  }

  const Type& DerefType(TypeRef ref) const {
    return std::get<Type>(DerefExpr(ref));
  }

  const Arena<Stmt>& arena_;
  const ControlFlowGraph& graph_;
  AbstractMachineState& state_;
  RegId next_reg_ = 1;
  std::size_t stack_offset_ = 0;
  std::map<std::string_view, std::size_t> var_stack_;
};

}  // namespace

void GenerateAbstractMachineFunction(const Arena<Stmt>& arena,
                                     const ControlFlowGraph& graph,
                                     AbstractMachineState& state) {
  AbstractMachineFunctionGenerator(arena, graph, state).Generate();
}

}  // namespace lucid
