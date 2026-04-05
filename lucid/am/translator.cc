#include "lucid/am/translator.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

class AbstractMachineFunctionGenerator {
 public:
  AbstractMachineFunctionGenerator(const SyntaxContext& ctx,
                                   const SyntaxControlFlowGraph& scfg,
                                   AbstractMachineState& state)
      : ctx_(ctx), scfg_(scfg), state_(state) {
    expr_and_stmt_to_reg_.resize(ctx_.Size());
  }

  AbstractMachineControlFlowGraph Generate() && {
    for (const auto& block : scfg_.blocks()) {
      graph_map_[block.ref] = am_cfg_.add().ref;
    }
    am_cfg_.first = graph_map_[scfg_.first];
    am_cfg_.last = graph_map_[scfg_.last];

    if (ctx_.DerefIdent(scfg_.func_name) == "printString") {
      auto& first_block = am_cfg_.add();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(Jump{
          .label = "_print_string",
      });
      first_block.instructions.push_back(SetReg32{
          .src_val = "0",
          .dst_reg = 0,
      });
      first_block.instructions.push_back(Return{});
      return std::move(am_cfg_);
    } else if (ctx_.DerefIdent(scfg_.func_name) == "sleep") {
      auto& first_block = am_cfg_.add();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(Jump{
          .label = "_sleep",
      });
      first_block.instructions.push_back(SetReg32{
          .src_val = "0",
          .dst_reg = 0,
      });
      first_block.instructions.push_back(Return{});
      return std::move(am_cfg_);
    }

    {
      auto& first_block = am_cfg_.get(am_cfg_.first);
      first_block.instructions.push_back(PushStack{});
      if (scfg_.has_func_calls) {
        for (std::size_t i = 12; i >= 1; --i) {
          first_block.instructions.push_back(StoreStack64{
              .offset = i - 1,
              .src_reg = RegId(i),
          });
          state_.stack_slots.push_back(8);
        }
      }
      for (RegId i = 0; i < scfg_.func_params.size(); ++i) {
        const auto& param = ctx_.DerefParam(scfg_.func_params[i]);
        const auto& param_type =
            std::get<BasicType>(ctx_.DerefType(param.type_constraint));
        std::string_view param_type_name = ctx_.DerefIdent(param_type.name);
        if (param_type_name == "Int32") {
          first_block.instructions.push_back(StoreStack32{
              .offset = state_.stack_slots.size(),
              .src_reg = static_cast<RegId>(i + 1),
          });
          var_stack_[param.name] = state_.stack_slots.size();
          state_.stack_slots.push_back(4);
        } else if (param_type_name == "Int64") {
          first_block.instructions.push_back(StoreStack64{
              .offset = state_.stack_slots.size(),
              .src_reg = static_cast<RegId>(i + 1),
          });
          var_stack_[param.name] = state_.stack_slots.size();
          state_.stack_slots.push_back(8);
        }
      }
    }

    for (const auto& block : scfg_.blocks()) {
      Process(block, am_cfg_.get(graph_map_[block.ref]));
    }

    {
      auto& last_block = am_cfg_.get(am_cfg_.last);
      if (scfg_.has_func_calls) {
        for (std::size_t i = 12; i >= 1; --i) {
          last_block.instructions.push_back(LoadStack64{
              .offset = i - 1,
              .dst_reg = RegId(i),
          });
          state_.stack_slots.push_back(8);
        }
      }
      last_block.instructions.push_back(PopStack{});
      last_block.instructions.push_back(Return{});
    }

    return std::move(am_cfg_);
  }

 private:
  void Process(const SyntaxControlFlowGraph::Block& block,
               AbstractMachineControlFlowGraph::Block& am_block) {
    am_block.instructions.push_back(Label{
        .id = block.ref.id(),
    });
    for (const auto& seq : block.sequences) {
      for (ExprRef expr : seq.expressions) {
        Process(expr, ctx_.DerefExpr(expr), am_block);
      }
      if (seq.stmt.has_value()) {
        Process(*seq.stmt, ctx_.DerefStmt(*seq.stmt), am_block);
      }
    }
    if (block.branch_cond != Arena<Expr>::kNullRef) {
      am_block.instructions.push_back(CondJump{
          .cond_reg = expr_and_stmt_to_reg_[block.branch_cond.id()],
          .then_label = scfg_.get(block.next[0]).ref.id(),
          .else_label = scfg_.get(block.next[1]).ref.id(),
      });
    } else if (block.next.size() == 1) {
      am_block.instructions.push_back(UncondJump{
          .label = scfg_.get(block.next[0]).ref.id(),
      });
    }
    for (const auto& next : block.next) {
      am_block.next.Insert(graph_map_[next]);
    }
    for (const auto& pred : block.preds) {
      am_block.preds.Insert(graph_map_[pred]);
    }
  }

  void Process(ExprRef ref, const Expr& expr,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::visit([&](auto&& expr) { ProcessExpr(ref, expr, am_block); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int64") {
      am_block.instructions.push_back(SetReg64{
          .src_val = ctx_.DerefIdent(expr.value),
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int32") {
      am_block.instructions.push_back(SetReg32{
          .src_val = ctx_.DerefIdent(expr.value),
          .dst_reg = reg,
      });
    }
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    RegId reg = next_reg_++;
    am_block.instructions.push_back(SetReg32{
        .src_val = ctx_.DerefIdent(expr.value) == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    auto string_id =
        reinterpret_cast<std::uintptr_t>(ctx_.DerefIdent(expr.value).data());
    state_.strings.insert({string_id, expr.value});

    RegId reg = next_reg_++;
    am_block.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    RegId reg = next_reg_++;

    FuncCall func_call = {
        .label = ctx_.DerefIdent(expr.func_name),
    };
    for (ExprRef arg : expr.args) {
      const auto& arg_expr = ctx_.DerefExpr(arg);
      auto arg_type = std::get<BasicType>(ctx_.DerefType(GetType(arg_expr)));
      std::string_view arg_type_name = ctx_.DerefIdent(arg_type.name);
      if (arg_type_name == "Int32") {
        func_call.args.push_back(FuncCall::Slot{
            .reg = expr_and_stmt_to_reg_[arg.id()],
            .bits = 32,
        });
      } else if (arg_type_name == "Int64" || arg_type_name == "String") {
        func_call.args.push_back(FuncCall::Slot{
            .reg = expr_and_stmt_to_reg_[arg.id()],
            .bits = 64,
        });
      }
    }
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32") {
      func_call.res = {
          .reg = reg,
          .bits = 32,
      };
    } else if (expr_type_name == "Int64") {
      func_call.res = {
          .reg = reg,
          .bits = 64,
      };
    }
    am_block.instructions.push_back(std::move(func_call));
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IdentExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(ctx_.DerefType(expr.type))) return;
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int32" || expr_type_name == "Bool") {
      am_block.instructions.push_back(LoadStack32{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      am_block.instructions.push_back(LoadStack64{
          .offset = var_stack_[expr.name],
          .dst_reg = reg,
      });
    }
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IndexExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    std::size_t base_offset = GetStackOffset(expr.base);
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = next_reg_++;
    if (expr_type_name == "Int32") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg32{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg64{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Bool") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg32{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    }
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(expr.lhs))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    auto reg = next_reg_++;
    switch (expr.op) {
      case BinaryOp::Add:
        if (expr_type_name == "Int32") {
          am_block.instructions.push_back(AddReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          am_block.instructions.push_back(AddReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Sub:
        if (expr_type_name == "Int64") {
          am_block.instructions.push_back(SubReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else {
          am_block.instructions.push_back(SubReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Mul:
        if (expr_type_name == "Int32") {
          am_block.instructions.push_back(MulReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          am_block.instructions.push_back(MulReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Div:
        if (expr_type_name == "Int32") {
          am_block.instructions.push_back(DivReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          am_block.instructions.push_back(DivReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Mod:
        if (expr_type_name == "Int32") {
          am_block.instructions.push_back(ModReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else if (expr_type_name == "Int64") {
          am_block.instructions.push_back(ModReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Gt:
        if (expr_type_name == "Int64") {
          am_block.instructions.push_back(GtReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else {
          am_block.instructions.push_back(GtReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Lt:
        if (expr_type_name == "Int64") {
          am_block.instructions.push_back(LtReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else {
          am_block.instructions.push_back(LtReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::Eq:
        if (expr_type_name == "Int64") {
          am_block.instructions.push_back(EqReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else {
          am_block.instructions.push_back(EqReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
      case BinaryOp::NotEq:
        if (expr_type_name == "Int64") {
          am_block.instructions.push_back(NotEqReg64{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        } else {
          am_block.instructions.push_back(NotEqReg32{
              .res_reg = reg,
              .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
              .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
          });
        }
        break;
    }
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void Process(StmtRef ref, const Stmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::visit([&](const auto& stmt) { Process(ref, stmt, am_block); }, stmt);
  }

  void Process(StmtRef ref, const ReturnStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto type = std::get<BasicType>(
        ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.value))));
    std::string_view type_name = ctx_.DerefIdent(type.name);
    if (type_name == "Int32" || type_name == "Bool") {
      am_block.instructions.push_back(MoveReg32{
          .src_reg = expr_and_stmt_to_reg_[stmt.value.id()],
          .dst_reg = 0,
      });
    } else if (type_name == "Int64") {
      am_block.instructions.push_back(MoveReg64{
          .src_reg = expr_and_stmt_to_reg_[stmt.value.id()],
          .dst_reg = 0,
      });
    }
  }

  void Process(StmtRef ref, const DoStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const VarAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32" || expr_type_name == "Bool") {
      am_block.instructions.push_back(StoreStack32{
          .offset = var_stack_[stmt.name],
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      am_block.instructions.push_back(StoreStack64{
          .offset = var_stack_[stmt.name],
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    }
  }

  void Process(StmtRef ref, const VarDeclStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(
            ctx_.DerefType(stmt.type_constraint))) {
      auto array_type =
          std::get<ArrayType>(ctx_.DerefType(stmt.type_constraint));
      const auto& var_decl_type = std::get<BasicType>(
          ctx_.DerefType(array_type.element_type_constraint));
      std::string_view var_decl_type_name = ctx_.DerefIdent(var_decl_type.name);
      auto size = std::atoi(ctx_.DerefIdent(array_type.size.value).data());
      auto stack_offset = state_.stack_slots.size();
      for (int i = 0; i < size; ++i) {
        if (var_decl_type_name == "Int32" || var_decl_type_name == "Bool") {
          state_.stack_slots.push_back(4);
        } else if (var_decl_type_name == "Int64") {
          state_.stack_slots.push_back(8);
        }
      }
      var_stack_[stmt.name] = stack_offset;
      return;
    }

    auto stmt_type = std::get<BasicType>(ctx_.DerefType(stmt.type_constraint));
    std::string_view stmt_type_name = ctx_.DerefIdent(stmt_type.name);
    auto stack_offset = state_.stack_slots.size();
    if (stmt_type_name == "Int32") {
      state_.stack_slots.push_back(4);
    } else if (stmt_type_name == "Int64") {
      state_.stack_slots.push_back(8);
    } else if (stmt_type_name == "Bool") {
      state_.stack_slots.push_back(4);
    }

    if (!stmt.init.has_value()) return;

    if (stmt_type_name == "Int32" || stmt_type_name == "Bool") {
      am_block.instructions.push_back(StoreStack32{
          .offset = stack_offset,
          .src_reg = expr_and_stmt_to_reg_[stmt.init->id()],
      });
      var_stack_[stmt.name] = stack_offset;
    } else if (stmt_type_name == "Int64") {
      am_block.instructions.push_back(StoreStack64{
          .offset = stack_offset,
          .src_reg = expr_and_stmt_to_reg_[stmt.init->id()],
      });
      var_stack_[stmt.name] = stack_offset;
    }
  }

  void Process(StmtRef ref, const ArrayAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    if (expr_type_name == "Int32") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg32{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg64{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Bool") {
      RegId offset_reg = next_reg_++;
      am_block.instructions.push_back(SetReg32{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg32{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg32{
          .offset = var_stack_[stmt.name],
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    }
  }

  void Process(StmtRef ref, const FuncDefStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const IfStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const LoopStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const BreakStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  std::size_t GetStackOffset(ExprRef expr_ref) {
    const auto& expr = std::get<IdentExpr>(ctx_.DerefExpr(expr_ref));
    return var_stack_[expr.name];
  }

  const SyntaxContext& ctx_;
  const SyntaxControlFlowGraph& scfg_;
  AbstractMachineState& state_;
  AbstractMachineControlFlowGraph am_cfg_;
  RegId next_reg_ = 1;
  std::unordered_map<StringIndex::Ref, std::size_t> var_stack_;
  std::unordered_map<SyntaxControlFlowGraph::BlockRef,
                     AbstractMachineControlFlowGraph::BlockRef>
      graph_map_;
  std::vector<RegId> expr_and_stmt_to_reg_;
};

}  // namespace

AbstractMachineControlFlowGraph GenerateAbstractMachineFunction(
    const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg,
    AbstractMachineState& state) {
  return AbstractMachineFunctionGenerator(ctx, scfg, state).Generate();
}

}  // namespace lucid
