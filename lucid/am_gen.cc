#include "lucid/am_gen.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "lucid/am.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/string_index.h"

namespace lucid {
namespace {

class AbstractMachineFunctionGenerator {
 public:
  AbstractMachineFunctionGenerator(const SyntaxContext& ctx,
                                   const ControlFlowGraph& graph,
                                   AbstractMachineState& state)
      : ctx_(ctx), graph_(graph), state_(state) {
    for (const auto& param_ref : graph.func_params) {
      const auto& param = ctx_.DerefParam(param_ref);
      // TODO: Handle `ArrayType`.
      const auto& param_type =
          std::get<BasicType>(ctx_.DerefType(param.type_constraint));
      std::string_view param_name = ctx.DerefIdent(param_type.name);
      if (param_name == "Int32") {
        state_.func.stack_slots.push_back(4);
      } else if (param_name == "Int64") {
        state_.func.stack_slots.push_back(8);
      } else if (param_name == "Bool") {
        state_.func.stack_slots.push_back(1);
      }
    }
    for (const auto& block : graph.blocks()) {
      for (const auto& seq : block.sequences) {
        if (seq.stmt.has_value()) {
          const auto& stmt = ctx_.DerefStmt(*seq.stmt);
          if (auto* var_decl = std::get_if<VarDeclStmt>(&stmt)) {
            if (auto* array_type = std::get_if<ArrayType>(
                    &ctx_.DerefType(var_decl->type_constraint))) {
              const auto& var_decl_type = std::get<BasicType>(
                  ctx_.DerefType(array_type->element_type_constraint));
              std::string_view var_decl_type_name =
                  ctx.DerefIdent(var_decl_type.name);
              auto size = std::atoi(ctx_.DerefIdent(array_type->size.value).data());
              for (int i = 0; i < size; ++i) {
                if (var_decl_type_name == "Int32" ||
                    var_decl_type_name == "Bool") {
                  state_.func.stack_slots.push_back(4);
                } else if (var_decl_type_name == "Int64") {
                  state_.func.stack_slots.push_back(8);
                }
              }
            } else {
              const auto& var_decl_type = std::get<BasicType>(
                  ctx_.DerefType(var_decl->type_constraint));
              std::string_view var_decl_type_name =
                  ctx.DerefIdent(var_decl_type.name);
              if (var_decl_type_name == "Int32") {
                state_.func.stack_slots.push_back(4);
              } else if (var_decl_type_name == "Int64") {
                state_.func.stack_slots.push_back(8);
              } else if (var_decl_type_name == "Bool") {
                state_.func.stack_slots.push_back(1);
              }
            }
          }
        }
      }
    }
  }

  void Generate() {
    if (graph_.has_func_calls) stack_offset_ = 12;

    state_.out_reg.clear();
    state_.out_reg.resize(ctx_.Size());

    state_.func.name = graph_.func_name;

    std::size_t instructions_count = 0;
    for (const auto& block : graph_.blocks()) {
      for (const auto& seq : block.sequences) {
        instructions_count += seq.expressions.size() + 1;
      }
    }
    state_.func.instructions.clear();
    state_.func.instructions.reserve(instructions_count * 2);

    if (ctx_.DerefIdent(graph_.func_name) == "printString") {
      state_.func.instructions.push_back(Jump{
          .label = "_print_string",
      });
      state_.func.instructions.push_back(SetReg32{
          .src_val = "0",
          .dst_reg = 0,
      });
      state_.func.instructions.push_back(Return{});
      return;
    } else if (ctx_.DerefIdent(graph_.func_name) == "sleep") {
      state_.func.instructions.push_back(Jump{
          .label = "_sleep",
      });
      state_.func.instructions.push_back(SetReg32{
          .src_val = "0",
          .dst_reg = 0,
      });
      state_.func.instructions.push_back(Return{});
      return;
    }

    std::ptrdiff_t push_pos, pop_pos;

    push_pos =
        state_.func.instructions.end() - state_.func.instructions.begin();
    for (RegId i = 0; i < graph_.func_params.size(); ++i) {
      const auto& param = ctx_.DerefParam(graph_.func_params[i]);
      const auto& param_type =
          std::get<BasicType>(ctx_.DerefType(param.type_constraint));
      std::string_view param_type_name = ctx_.DerefIdent(param_type.name);
      if (param_type_name == "Int32") {
        state_.func.instructions.push_back(StoreStack32{
            .offset = stack_offset_,
            .src_reg = static_cast<RegId>(i + 1),
        });
        var_stack_[param.name] = stack_offset_;
        ++stack_offset_;
      } else if (param_type_name == "Int64") {
        state_.func.instructions.push_back(StoreStack64{
            .offset = stack_offset_,
            .src_reg = static_cast<RegId>(i + 1),
        });
        var_stack_[param.name] = stack_offset_;
        ++stack_offset_;
      }
    }

    RegId highest_reg = 1;
    for (const auto& block : graph_.blocks()) {
      if (next_reg_ > highest_reg) highest_reg = next_reg_;
      next_reg_ = 1;

      state_.func.instructions.push_back(Label{
          .id = block.ref.id(),
      });

      if (block.ref == graph_.get(graph_.last).ref) {
        pop_pos =
            state_.func.instructions.end() - state_.func.instructions.begin();
        state_.func.instructions.push_back(Return{});
      }

      for (const auto& seq : block.sequences) {
        for (const auto& expr_ref : seq.expressions) {
          Process(expr_ref, ctx_.DerefExpr(expr_ref));
        }
        if (seq.stmt.has_value()) {
          Process(*seq.stmt, ctx_.DerefStmt(*seq.stmt));
        }
      }

      if (block.branch_cond != Arena<Expr>::kNullRef) {
        state_.func.instructions.push_back(CondJump{
            .cond_reg = state_.out_reg[block.branch_cond.id()],
            .then_label = graph_.get(block.next[0]).ref.id(),
            .else_label = graph_.get(block.next[1]).ref.id(),
        });
      } else if (block.next.size() == 1) {
        state_.func.instructions.push_back(UncondJump{
            .label = graph_.get(block.next[0]).ref.id(),
        });
      }
    }
    next_reg_ = highest_reg;

    state_.func.instructions.insert(state_.func.instructions.begin() + pop_pos,
                                    PopStack{});
    if (graph_.has_func_calls) {
      for (std::size_t i = 1; i <= 12; ++i) {
        state_.func.instructions.insert(
            state_.func.instructions.begin() + pop_pos, LoadStack64{
                                                            .offset = i - 1,
                                                            .dst_reg = RegId(i),
                                                        });
        state_.func.stack_slots.push_back(8);
      }
      for (std::size_t i = 1; i <= 12; ++i) {
        state_.func.instructions.insert(
            state_.func.instructions.begin() + push_pos,
            StoreStack64{
                .offset = i - 1,
                .src_reg = RegId(i),
            });
      }
    }
    state_.func.instructions.insert(state_.func.instructions.begin() + push_pos,
                                    PushStack{});
  }

 private:
  void Process(StmtRef ref, const Stmt& stmt) {
    std::visit([this, ref](const auto& stmt) { Process(ref, stmt); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt) {
    auto type = std::get<BasicType>(
        ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.value))));
    std::string_view type_name = ctx_.DerefIdent(type.name);
    if (type_name == "Int32" || type_name == "Bool") {
      state_.func.instructions.push_back(MoveReg32{
          .src_reg = state_.out_reg[stmt.value.id()],
          .dst_reg = 0,
      });
    } else if (type_name == "Int64") {
      state_.func.instructions.push_back(MoveReg64{
          .src_reg = state_.out_reg[stmt.value.id()],
          .dst_reg = 0,
      });
    }
  }

  void Process(StmtRef ref, const DoStmt& stmt) {}

  void Process(ExprRef ref, const Expr& expr) {
    std::visit([this, ref](auto&& expr) { ProcessExpr(ref, expr); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr) {
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int64") {
      state_.func.instructions.push_back(SetReg64{
          .src_val = ctx_.DerefIdent(expr.value),
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int32") {
      state_.func.instructions.push_back(SetReg32{
          .src_val = ctx_.DerefIdent(expr.value),
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr) {
    RegId reg = next_reg_++;
    state_.func.instructions.push_back(SetReg32{
        .src_val = ctx_.DerefIdent(expr.value) == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr) {
    auto string_id = reinterpret_cast<std::uintptr_t>(ctx_.DerefIdent(expr.value).data());
    state_.strings.insert({string_id, expr.value});

    RegId reg = next_reg_++;
    state_.func.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr) {
    RegId reg = next_reg_++;
    RegId i = 1;
    for (ExprRef arg : expr.args) {
      const auto& arg_expr = ctx_.DerefExpr(arg);
      auto arg_type = std::get<BasicType>(ctx_.DerefType(GetType(arg_expr)));
      std::string_view arg_type_name = ctx_.DerefIdent(arg_type.name);
      if (arg_type_name == "Int32") {
        state_.func.instructions.push_back(MoveReg32{
            .src_reg = state_.out_reg[arg.id()],
            .dst_reg = i++,
        });
      } else if (arg_type_name == "Int64" || arg_type_name == "String") {
        state_.func.instructions.push_back(MoveReg64{
            .src_reg = state_.out_reg[arg.id()],
            .dst_reg = i++,
        });
      }
    }
    state_.func.instructions.push_back(Jump{
        .label = ctx_.DerefIdent(expr.func_name),
    });
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32") {
      state_.func.instructions.push_back(MoveReg32{
          .src_reg = 0,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      state_.func.instructions.push_back(MoveReg64{
          .src_reg = 0,
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref.id()] = reg;
  }

  void Process(StmtRef stmt_ref, const VarDeclStmt& stmt) {
    if (std::holds_alternative<ArrayType>(
            ctx_.DerefType(stmt.type_constraint))) {
      auto array_type =
          std::get<ArrayType>(ctx_.DerefType(stmt.type_constraint));
      auto size = std::atoi(ctx_.DerefIdent(array_type.size.value).data());
      var_stack_[stmt.name] = stack_offset_;
      for (int i = 0; i < size; ++i) {
        ++stack_offset_;
      }
      return;
    }

    if (!stmt.init.has_value()) return;

    auto stmt_type = std::get<BasicType>(ctx_.DerefType(stmt.type_constraint));
    std::string_view stmt_type_name = ctx_.DerefIdent(stmt_type.name);
    if (stmt_type_name == "Int32") {
      state_.func.instructions.push_back(StoreStack32{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init->id()],
      });
      var_stack_[stmt.name] = stack_offset_;
      ++stack_offset_;
    } else if (stmt_type_name == "Int64") {
      state_.func.instructions.push_back(StoreStack64{
          .offset = stack_offset_,
          .src_reg = state_.out_reg[stmt.init->id()],
      });
      var_stack_[stmt.name] = stack_offset_;
      ++stack_offset_;
    }
  }

  void Process(StmtRef stmt_ref, const VarAssignStmt& stmt) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32") {
      state_.func.instructions.push_back(StoreStack32{
          .offset = var_stack_[stmt.name],
          .src_reg = state_.out_reg[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      state_.func.instructions.push_back(StoreStack64{
          .offset = var_stack_[stmt.name],
          .src_reg = state_.out_reg[stmt.expr.id()],
      });
    }
  }

  void Process(StmtRef stmt_ref, const ArrayAssignStmt& stmt) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[stmt.index.id()],
      });

      state_.func.instructions.push_back(StoreStackReg32{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = state_.out_reg[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[stmt.index.id()],
      });

      state_.func.instructions.push_back(StoreStackReg64{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = state_.out_reg[stmt.expr.id()],
      });
    } else if (expr_type_name == "Bool") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[stmt.index.id()],
      });

      state_.func.instructions.push_back(StoreStackReg32{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = state_.out_reg[stmt.expr.id()],
      });
    }
  }

  void Process(StmtRef stmt_ref, const BreakStmt& stmt) {}

  void ProcessExpr(ExprRef ref, const IdentExpr& expr) {
    if (std::holds_alternative<ArrayType>(ctx_.DerefType(expr.type))) return;
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int32") {
      state_.func.instructions.push_back(LoadStack32{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      state_.func.instructions.push_back(LoadStack64{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IndexExpr& expr) {
    std::size_t base_offset = GetStackOffset(expr.base);
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int32") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[expr.index.id()],
      });

      state_.func.instructions.push_back(LoadStackReg32{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[expr.index.id()],
      });

      state_.func.instructions.push_back(LoadStackReg64{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Bool") {
      RegId offset_reg = next_reg_++;
      state_.func.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      state_.func.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = state_.out_reg[expr.index.id()],
      });

      state_.func.instructions.push_back(LoadStackReg32{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    }
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(expr.lhs))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        if (expr_type_name == "Int32") {
          state_.func.instructions.push_back(AddReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(AddReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Sub:
        if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(SubReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else {
          state_.func.instructions.push_back(SubReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Mul:
        if (expr_type_name == "Int32") {
          state_.func.instructions.push_back(MulReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(MulReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Div:
        if (expr_type_name == "Int32") {
          state_.func.instructions.push_back(DivReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(DivReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Mod:
        if (expr_type_name == "Int32") {
          state_.func.instructions.push_back(ModReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(ModReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Gt:
        if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(GtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else {
          state_.func.instructions.push_back(GtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Lt:
        if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(LtReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else {
          state_.func.instructions.push_back(LtReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Eq:
        if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(EqReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else {
          state_.func.instructions.push_back(EqReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::NotEq:
        if (expr_type_name == "Int64") {
          state_.func.instructions.push_back(NotEqReg64{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        } else {
          state_.func.instructions.push_back(NotEqReg32{
              .res_reg = reg,
              .lhs_reg = state_.out_reg[expr.lhs.id()],
              .rhs_reg = state_.out_reg[expr.rhs.id()],
          });
        }
        break;
    }
    state_.out_reg[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const Type& expr) {
    // TODO: How to generate code for types?
  }

  std::size_t GetStackOffset(ExprRef expr_ref) {
    const auto& expr = std::get<IdentExpr>(ctx_.DerefExpr(expr_ref));
    return var_stack_[expr.name];
  }

  const SyntaxContext& ctx_;
  const ControlFlowGraph& graph_;
  AbstractMachineState& state_;
  RegId next_reg_ = 1;
  std::size_t stack_offset_ = 0;
  std::unordered_map<StringIndex::Ref, std::size_t> var_stack_;
};

}  // namespace

void GenerateAbstractMachineFunction(const SyntaxContext& ctx,
                                     const ControlFlowGraph& graph,
                                     AbstractMachineState& state) {
  AbstractMachineFunctionGenerator(ctx, graph, state).Generate();
}

}  // namespace lucid
