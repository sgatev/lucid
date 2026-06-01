#include "lucid/am/translator.h"

#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/vm/interpreter.h"

namespace lucid {
namespace {

class AbstractMachineFunctionGenerator {
 public:
  AbstractMachineFunctionGenerator(
      const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
      const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg,
      AbstractMachineState& am_state)
      : syn_ctx_(syn_ctx),
        syn_cfg_(syn_cfg),
        am_state_(am_state),
        vm_(am_cfgs, am_state) {
    expr_and_stmt_to_reg_.resize(syn_ctx_.Size());
  }

  AbstractMachineControlFlowGraph Generate() && {
    for (const auto& block : syn_cfg_.blocks()) {
      graph_map_.Set(block.ref, am_cfg_.AddBlock().ref);
    }

    auto scfg_first = graph_map_.Get(syn_cfg_.first);
    assert(scfg_first.has_value());
    am_cfg_.first = *scfg_first;

    auto scfg_last = graph_map_.Get(syn_cfg_.last);
    assert(scfg_last.has_value());
    am_cfg_.last = *scfg_last;

    result_reg_ = {am_cfg_.next_free_reg_id++,
                   GetRegSize(syn_cfg_.func_result_type)};

    if (syn_ctx_.DerefIdent(syn_cfg_.func_name) == "printString") {
      auto& first_block = am_cfg_.AddBlock();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(FuncCall{
          .label = "_print_string",
      });
      first_block.instructions.push_back(SetReg{
          .src_val = 0,
          .dst_reg = result_reg_,
      });
      first_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
      return std::move(am_cfg_);
    } else if (syn_ctx_.DerefIdent(syn_cfg_.func_name) == "sleep") {
      auto& first_block = am_cfg_.AddBlock();
      am_cfg_.first = first_block.ref;

      first_block.instructions.push_back(FuncCall{
          .label = "_sleep",
      });
      first_block.instructions.push_back(SetReg{
          .src_val = 0,
          .dst_reg = result_reg_,
      });
      first_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
      return std::move(am_cfg_);
    }

    {
      for (const auto& param_ref : syn_cfg_.func_params) {
        const auto& param = syn_ctx_.DerefParam(param_ref);
        am_cfg_.params.push_back(GetVarReg(param.name, param.type_constraint));
      }
    }

    for (const auto& block : syn_cfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      assert(am_cfg_block_ref.has_value());
      Process(block, am_cfg_.GetBlock(*am_cfg_block_ref));
    }
    for (const auto& block : syn_cfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      assert(am_cfg_block_ref.has_value());
      auto& am_block = am_cfg_.GetBlock(*am_cfg_block_ref);
      for (auto phi_ref : block.phis) {
        const auto& phi = syn_cfg_.deref(phi_ref);

        auto& am_phi = am_block.phis.emplace_back();
        am_phi.dst = GetVarReg(phi.name, phi.type_constraint);
        for (const auto& arg : phi.args) {
          am_phi.srcs.push_back(GetVarReg(arg, phi.type_constraint));
        }
      }
    }

    {
      auto& last_block = am_cfg_.GetBlock(am_cfg_.last);
      last_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
    }

    return std::move(am_cfg_);
  }

 private:
  void Process(const SyntaxControlFlowGraph::Block& block,
               AbstractMachineControlFlowGraph::Block& am_block) {
    for (const auto& seq : block.sequences) {
      for (ExprRef expr_ref : seq.expressions) {
        const Expr& expr = syn_ctx_.DerefExpr(expr_ref);
        Process(expr_ref, expr, am_block);

        bool is_comp =
            std::visit([](const auto& expr) { return expr.is_comp; }, expr);
        if (is_comp) {
          am_block.instructions.back() =
              vm_.Interpret(am_block.instructions.back());
        }
      }
      if (seq.stmt.has_value()) {
        const auto& stmt = syn_ctx_.DerefStmt(*seq.stmt);
        Process(*seq.stmt, stmt, am_block);

        bool is_comp = false;
        if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          is_comp = var_decl_stmt->is_comp;
        }
        if (is_comp) {
          am_block.instructions.back() =
              vm_.Interpret(am_block.instructions.back());
        }
      }
    }
    if (block.branch_cond != Arena<Expr>::kNullRef) {
      am_block.branch_cond = expr_and_stmt_to_reg_[block.branch_cond.id()];
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
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = expr.value,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = syn_ctx_.DerefIdent(expr.value) == "true" ? 1 : 0,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    auto string_id = reinterpret_cast<std::uintptr_t>(
        syn_ctx_.DerefIdent(expr.value).data());
    am_state_.strings.Insert(string_id, expr.value);

    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    FuncCall func_call = {
        .label = syn_ctx_.DerefIdent(expr.func_name),
    };
    for (ExprRef arg : expr.args) {
      func_call.args.push_back(FuncCall::Slot{
          .reg = expr_and_stmt_to_reg_[arg.id()],
      });
    }
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    func_call.res = {
        .reg = reg,
    };
    am_block.instructions.push_back(std::move(func_call));
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IdentExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(syn_ctx_.DerefType(expr.type)))
      return;
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(MoveReg{
        .src_reg = GetVarReg(expr.name, expr.type),
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IndexExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    std::size_t base_offset = GetStackOffset(expr.base);
    auto expr_type = std::get<BasicType>(syn_ctx_.DerefType(expr.type));
    std::string_view expr_type_name = syn_ctx_.DerefIdent(expr_type.name);
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    if (expr_type_name == "Int32") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 4,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Int64") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 8,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    } else if (expr_type_name == "Bool") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 4,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[expr.index.id()],
      });

      am_block.instructions.push_back(LoadStackReg{
          .offset = base_offset,
          .offset_reg = offset_reg,
          .dst_reg = reg,
      });
    }
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
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
        am_block.instructions.push_back(DivReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Mod:
        am_block.instructions.push_back(ModReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Gt:
        am_block.instructions.push_back(GtReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Lt:
        am_block.instructions.push_back(LtReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Eq:
        am_block.instructions.push_back(EqReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::NotEq:
        am_block.instructions.push_back(NotEqReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
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
    auto expr_type_ref = GetType(syn_ctx_.DerefExpr(stmt.expr));
    am_block.instructions.push_back(MoveReg{
        .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
        .dst_reg = GetVarReg(stmt.name, expr_type_ref),
    });
  }

  void Process(StmtRef ref, const VarDeclStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(
            syn_ctx_.DerefType(stmt.type_constraint))) {
      auto array_type =
          std::get<ArrayType>(syn_ctx_.DerefType(stmt.type_constraint));
      const auto& var_decl_type = std::get<BasicType>(
          syn_ctx_.DerefType(array_type.element_type_constraint));
      std::string_view var_decl_type_name =
          syn_ctx_.DerefIdent(var_decl_type.name);
      std::size_t size = array_type.size.value;
      auto stack_offset = am_cfg_.stack_slots.size();
      for (int i = 0; i < size; ++i) {
        if (var_decl_type_name == "Int32" || var_decl_type_name == "Bool") {
          am_cfg_.stack_slots.push_back(4);
        } else if (var_decl_type_name == "Int64") {
          am_cfg_.stack_slots.push_back(8);
        }
      }
      var_stack_.Set(stmt.name, stack_offset);
      return;
    }

    if (stmt.init.has_value()) {
      am_block.instructions.push_back(MoveReg{
          .src_reg = expr_and_stmt_to_reg_[stmt.init->id()],
          .dst_reg = GetVarReg(stmt.name, stmt.type_constraint),
      });
    }
  }

  void Process(StmtRef ref, const ArrayAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type = std::get<BasicType>(
        syn_ctx_.DerefType(GetType(syn_ctx_.DerefExpr(stmt.expr))));
    std::string_view expr_type_name = syn_ctx_.DerefIdent(expr_type.name);
    auto stmt_offset = var_stack_.Get(stmt.name);
    assert(stmt_offset.has_value());
    if (expr_type_name == "Int32") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 4,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg{
          .offset = *stmt_offset,
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Int64") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 8,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg{
          .offset = *stmt_offset,
          .offset_reg = offset_reg,
          .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
      });
    } else if (expr_type_name == "Bool") {
      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 4,
          .dst_reg = offset_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = offset_reg,
          .lhs_reg = offset_reg,
          .rhs_reg = expr_and_stmt_to_reg_[stmt.index.id()],
      });

      am_block.instructions.push_back(StoreStackReg{
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
    const auto& expr = std::get<IdentExpr>(syn_ctx_.DerefExpr(expr_ref));
    auto pos = var_stack_.Get(expr.name);
    assert(pos.has_value());
    return *pos;
  }

  Reg GetVarReg(StringIndex::Ref var_name, TypeRef var_type) {
    auto it = var_to_reg_.Get(var_name);
    if (it.has_value()) return *it;

    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(var_type)};
    var_to_reg_.Insert(var_name, reg);
    return reg;
  }

  RegSize GetRegSize(TypeRef type_ref) {
    auto expr_type = std::get<BasicType>(syn_ctx_.DerefType(type_ref));
    std::string_view type_name = syn_ctx_.DerefIdent(expr_type.name);
    if (type_name == "Int64" || type_name == "String") {
      return RegSize::RegSize64;
    }
    return RegSize::RegSize32;
  }

  const SyntaxContext& syn_ctx_;
  const SyntaxControlFlowGraph& syn_cfg_;
  AbstractMachineState& am_state_;
  Interpreter vm_;
  AbstractMachineControlFlowGraph am_cfg_;
  Reg result_reg_;
  HashMap<StringIndex::Ref, Reg> var_to_reg_;
  HashMap<StringIndex::Ref, std::size_t> var_stack_;
  HashMap<SyntaxControlFlowGraph::BlockRef,
          AbstractMachineControlFlowGraph::BlockRef>
      graph_map_;
  std::vector<Reg> expr_and_stmt_to_reg_;
};

}  // namespace

AbstractMachineControlFlowGraph GenerateAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg,
    AbstractMachineState& am_state) {
  return AbstractMachineFunctionGenerator(am_cfgs, syn_ctx, syn_cfg, am_state)
      .Generate();
}

}  // namespace lucid
