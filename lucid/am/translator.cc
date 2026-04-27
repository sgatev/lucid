#include "lucid/am/translator.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_map.h"
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
      graph_map_.Set(block.ref, am_cfg_.add().ref);
    }

    auto scfg_first = graph_map_.Get(scfg_.first);
    assert(scfg_first.has_value());
    am_cfg_.first = *scfg_first;

    auto scfg_last = graph_map_.Get(scfg_.last);
    assert(scfg_last.has_value());
    am_cfg_.last = *scfg_last;

    result_reg_ = {next_reg_id_++, GetRegSize(scfg_.func_result_type)};

    if (ctx_.DerefIdent(scfg_.func_name) == "printString") {
      auto& first_block = am_cfg_.add();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(PushStack{});
      first_block.instructions.push_back(Jump{
          .label = "_print_string",
      });
      first_block.instructions.push_back(SetReg{
          .src_val = "0",
          .dst_reg = result_reg_,
      });
      first_block.instructions.push_back(PopStack{});
      first_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
      return std::move(am_cfg_);
    } else if (ctx_.DerefIdent(scfg_.func_name) == "sleep") {
      auto& first_block = am_cfg_.add();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(PushStack{});
      first_block.instructions.push_back(Jump{
          .label = "_sleep",
      });
      first_block.instructions.push_back(SetReg{
          .src_val = "0",
          .dst_reg = result_reg_,
      });
      first_block.instructions.push_back(PopStack{});
      first_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
      return std::move(am_cfg_);
    }

    {
      auto& first_block = am_cfg_.get(am_cfg_.first);
      first_block.instructions.push_back(PushStack{});
      for (int i = 0; i < scfg_.func_params.size(); ++i) {
        const auto& param = ctx_.DerefParam(scfg_.func_params[i]);
        const auto& param_type =
            std::get<BasicType>(ctx_.DerefType(param.type_constraint));
        std::string_view param_type_name = ctx_.DerefIdent(param_type.name);
        if (param_type_name == "Int32" || param_type_name == "Bool") {
          am_cfg_.params.push_back(
              {.bits = 32,
               .reg = GetVarReg(param.name, param.type_constraint)});
        } else if (param_type_name == "Int64" || param_type_name == "String") {
          am_cfg_.params.push_back(
              {.bits = 64,
               .reg = GetVarReg(param.name, param.type_constraint)});
        }
      }
    }

    for (const auto& block : scfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      assert(am_cfg_block_ref.has_value());
      Process(block, am_cfg_.get(*am_cfg_block_ref));
    }
    for (const auto& block : scfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      assert(am_cfg_block_ref.has_value());
      auto& am_block = am_cfg_.get(*am_cfg_block_ref);
      for (auto phi_ref : block.phis) {
        const auto& phi = scfg_.deref(phi_ref);

        auto& am_phi = am_block.phis.emplace_back();
        am_phi.target = GetVarReg(phi.name, phi.type_constraint);
        for (const auto& arg : phi.args) {
          am_phi.sources.push_back(GetVarReg(arg, phi.type_constraint));
        }
      }
    }

    {
      auto& last_block = am_cfg_.get(am_cfg_.last);
      last_block.instructions.push_back(PopStack{});
      last_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
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
      auto am_cfg_next = graph_map_.Get(next);
      assert(am_cfg_next.has_value());
      am_block.next.push_back(*am_cfg_next);
    }
    for (const auto& pred : block.preds) {
      auto am_cfg_pred = graph_map_.Get(pred);
      assert(am_cfg_pred.has_value());
      am_block.preds.push_back(*am_cfg_pred);
    }
  }

  void Process(ExprRef ref, const Expr& expr,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::visit([&](auto&& expr) { ProcessExpr(ref, expr, am_block); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = ctx_.DerefIdent(expr.value),
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = ctx_.DerefIdent(expr.value) == "true" ? "1" : "0",
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    auto string_id =
        reinterpret_cast<std::uintptr_t>(ctx_.DerefIdent(expr.value).data());
    state_.strings.Insert(string_id, expr.value);

    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    FuncCall func_call = {
        .label = ctx_.DerefIdent(expr.func_name),
    };
    for (ExprRef arg : expr.args) {
      func_call.args.push_back(FuncCall::Slot{
          .reg = expr_and_stmt_to_reg_[arg.id()],
      });
    }
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    func_call.res = {
        .reg = reg,
    };
    am_block.instructions.push_back(std::move(func_call));
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IdentExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(ctx_.DerefType(expr.type))) return;
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    am_block.instructions.push_back(MoveReg{
        .src_reg = GetVarReg(expr.name, expr.type),
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IndexExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    std::size_t base_offset = GetStackOffset(expr.base);
    auto expr_type = std::get<BasicType>(ctx_.DerefType(expr.type));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    if (expr_type_name == "Int32") {
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
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
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
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
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
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
    RegId reg = {next_reg_id_++, GetRegSize(expr.type)};
    switch (expr.op) {
      case BinaryOp::Add:
        am_block.instructions.push_back(AddReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Sub:
        am_block.instructions.push_back(SubReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Mul:
        am_block.instructions.push_back(MulReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
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
    am_block.instructions.push_back(MoveReg{
        .src_reg = expr_and_stmt_to_reg_[stmt.value.id()],
        .dst_reg = result_reg_,
    });
  }

  void Process(StmtRef ref, const DoStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::get<FuncCall>(am_block.instructions.back()).res.reset();
  }

  void Process(StmtRef ref, const VarAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type_ref = GetType(ctx_.DerefExpr(stmt.expr));
    am_block.instructions.push_back(MoveReg{
        .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
        .dst_reg = GetVarReg(stmt.name, expr_type_ref),
    });
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
      var_stack_.Set(stmt.name, stack_offset);
      return;
    }

    if (!stmt.init.has_value()) return;

    am_block.instructions.push_back(MoveReg{
        .src_reg = expr_and_stmt_to_reg_[stmt.init->id()],
        .dst_reg = GetVarReg(stmt.name, stmt.type_constraint),
    });
  }

  void Process(StmtRef ref, const ArrayAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type =
        std::get<BasicType>(ctx_.DerefType(GetType(ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = ctx_.DerefIdent(expr_type.name);
    auto stmt_offset = var_stack_.Get(stmt.name);
    assert(stmt_offset.has_value());
    if (expr_type_name == "Int32") {
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg32{
          .offset = *stmt_offset,
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "8",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg64{
          .offset = *stmt_offset,
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Bool") {
      RegId offset_reg = {next_reg_id_++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = "4",
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg32{
          .offset = *stmt_offset,
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
    auto pos = var_stack_.Get(expr.name);
    assert(pos.has_value());
    std::size_t offset = 0;
    for (int i = 0; i < *pos; ++i) offset += state_.stack_slots[i];
    return offset;
  }

  RegId GetVarReg(StringIndex::Ref var_name, TypeRef var_type) {
    auto it = var_to_reg_.Get(var_name);
    if (it.has_value()) return *it;

    RegId reg = {next_reg_id_++, GetRegSize(var_type)};
    var_to_reg_.Insert(var_name, reg);
    return reg;
  }

  RegSize GetRegSize(TypeRef type_ref) {
    auto expr_type = std::get<BasicType>(ctx_.DerefType(type_ref));
    std::string_view type_name = ctx_.DerefIdent(expr_type.name);
    if (type_name == "Int64" || type_name == "String") {
      return RegSize::RegSize64;
    }
    return RegSize::RegSize32;
  }

  const SyntaxContext& ctx_;
  const SyntaxControlFlowGraph& scfg_;
  AbstractMachineState& state_;
  AbstractMachineControlFlowGraph am_cfg_;
  std::int32_t next_reg_id_ = 1;
  RegId result_reg_;
  HashMap<StringIndex::Ref, RegId> var_to_reg_;
  HashMap<StringIndex::Ref, std::size_t> var_stack_;
  HashMap<SyntaxControlFlowGraph::BlockRef,
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
