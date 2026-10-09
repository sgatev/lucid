#include "lucid/am/translator.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/context.h"
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
        vm_(am_cfgs, am_state, am_cfg_.stack_slots, /*depth=*/0, comp_steps_),
        expr_to_reg_(am_state.expr_regs) {
    expr_to_reg_.resize(syn_ctx_.Size());
  }

  std::expected<AbstractMachineControlFlowGraph, CompError> Generate() && {
    result_reg_ = {am_cfg_.next_free_reg_id++,
                   GetRegSize(syn_cfg_.func_result_type)};
    am_cfg_.result_kind = KindOf(syn_cfg_.func_result_type);

    if (syn_ctx_.DerefIdent(syn_cfg_.func_name) == "printString") {
      auto& first_block = am_cfg_.AddBlock();
      am_cfg_.first = first_block.ref;
      am_cfg_.last = first_block.ref;

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
      am_cfg_.last = first_block.ref;

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

    // Only the blocks control can come to are carried over, with the edges
    // between them. A block nothing reaches is never run, and leaving it out
    // keeps every block dominated by the first, which colouring the
    // registers asks of the graph. It is also not renamed into single
    // assignment form, so what it names would not be registers written once.
    const std::vector<bool> reached = ReachedBlocks(syn_cfg_);
    for (const auto& block : syn_cfg_.blocks()) {
      if (!reached[block.ref.id()]) continue;
      graph_map_.Set(block.ref, am_cfg_.AddBlock().ref);
    }
    am_cfg_.first = graph_map_.Get(syn_cfg_.first).value();
    // A function that never ends has no way to its last block.
    if (reached[syn_cfg_.last.id()]) {
      am_cfg_.last = graph_map_.Get(syn_cfg_.last).value();
    }

    {
      for (const auto& param_ref : syn_cfg_.func_params) {
        const auto& param = syn_ctx_.DerefParam(param_ref);
        am_cfg_.params.push_back(GetVarReg(param.name, param.type_constraint));
      }
    }

    for (const auto& block : syn_cfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      if (!am_cfg_block_ref.has_value()) continue;
      std::expected<void, CompError> res =
          Process(block, am_cfg_.GetBlock(*am_cfg_block_ref));
      if (!res.has_value()) return std::unexpected(res.error());
    }
    for (const auto& block : syn_cfg_.blocks()) {
      auto am_cfg_block_ref = graph_map_.Get(block.ref);
      if (!am_cfg_block_ref.has_value()) continue;
      auto& am_block = am_cfg_.GetBlock(*am_cfg_block_ref);
      for (auto phi_ref : block.phis) {
        const auto& phi = syn_cfg_.deref(phi_ref);

        // A phi function takes an argument along each way in, in the order
        // of the block's predecessors, and a way in from a block nothing
        // reaches is left out along with it.
        auto& am_phi = am_block.phis.emplace_back();
        am_phi.dst = GetVarReg(phi.name, phi.type_constraint);
        for (std::size_t i = 0; i < phi.args.size(); ++i) {
          if (!reached[block.preds[i].id()]) continue;
          am_phi.srcs.push_back(GetVarReg(phi.args[i], phi.type_constraint));
        }
      }
    }

    if (am_cfg_.last != AbstractMachineControlFlowGraph::kNullBlockRef) {
      // What the function returns is settled where it ends, by a phi
      // function taking each way in the value returned along it, so that the
      // register holding it is written in one place only. A way in that
      // returns nothing, because the function has no result, is given a
      // placeholder: the checks before this have made sure no other can.
      auto& last_block = am_cfg_.GetBlock(am_cfg_.last);
      if (!last_block.preds.empty()) {
        auto& result_phi = last_block.phis.emplace_back();
        result_phi.dst = result_reg_;
        for (const auto pred : last_block.preds) {
          if (pred.id() < returned_regs_.size() &&
              returned_regs_[pred.id()].has_value()) {
            result_phi.srcs.push_back(returned_regs_[pred.id()].value());
            continue;
          }
          const Reg placeholder = {am_cfg_.next_free_reg_id++,
                                   result_reg_.size};
          am_cfg_.GetBlock(pred).instructions.push_back(SetReg{
              .src_val = 0,
              .dst_reg = placeholder,
          });
          result_phi.srcs.push_back(placeholder);
        }
      }
      last_block.instructions.push_back(Return{
          .res_reg = result_reg_,
      });
    }

    return std::move(am_cfg_);
  }

 private:
  std::expected<void, CompError> Process(
      const SyntaxControlFlowGraph::Block& block,
      AbstractMachineControlFlowGraph::Block& am_block) {
    for (const auto& seq : block.sequences) {
      for (ExprRef expr_ref : seq.expressions) {
        const Expr& expr = syn_ctx_.DerefExpr(expr_ref);
        const std::size_t emitted = am_block.instructions.size();
        Process(expr_ref, expr, am_block);

        // What an expression comes to is worked out by running the
        // instruction it ends in. One that ends in none, such as a read of a
        // variable, is in a register that holds it already.
        bool is_comp =
            std::visit([](const auto& expr) { return expr.is_comp; }, expr);
        if (is_comp && am_block.instructions.size() > emitted) {
          std::expected<Instruction, CompError> res =
              vm_.Interpret(am_block.instructions.back());
          if (!res.has_value()) return std::unexpected(res.error());

          am_block.instructions.back() = std::move(*res);
        }
      }
      if (const auto& stmt_ref = seq.stmt; stmt_ref.has_value()) {
        const auto& stmt = syn_ctx_.DerefStmt(*stmt_ref);
        Process(*stmt_ref, stmt, am_block);

        bool is_comp = false;
        if (const auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
          is_comp = var_decl_stmt->is_comp;
        }
        if (is_comp) {
          std::expected<Instruction, CompError> res =
              vm_.Interpret(am_block.instructions.back());
          if (!res.has_value()) return std::unexpected(res.error());

          am_block.instructions.back() = std::move(*res);
        }
      }
    }
    if (block.branch_cond != Arena<Expr>::kNullRef) {
      am_block.branch_cond = expr_to_reg_[block.branch_cond.id()];
    }
    for (const auto& succ : block.succs) {
      auto am_cfg_succ = graph_map_.Get(succ);
      assert(am_cfg_succ.has_value());
      am_block.succs.push_back(*am_cfg_succ);
    }
    for (const auto& pred : block.preds) {
      auto am_cfg_pred = graph_map_.Get(pred);
      if (!am_cfg_pred.has_value()) continue;
      am_block.preds.push_back(*am_cfg_pred);
    }
    return {};
  }

  void Process(ExprRef ref, const Expr& expr,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::visit([&](auto&& expr) { ProcessExpr(ref, expr, am_block); }, expr);
  }

  void ProcessExpr(ExprRef ref, const IntLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetValue(expr.value, reg, am_state_));
    expr_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = expr.value ? 1 : 0,
        .dst_reg = reg,
    });
    expr_to_reg_[ref.id()] = reg;
  }

  // Returns the index that identifies `ref` in the string constant pool,
  // adding it to the pool if it is not already there.
  //
  // Equal strings share one reference, so this also deduplicates them.
  std::uint32_t AddString(StringIndex::Ref ref) {
    auto& strings = am_state_.strings;
    auto it = std::ranges::find(strings, ref);
    if (it == strings.end()) it = strings.insert(it, ref);
    return static_cast<std::uint32_t>(it - strings.begin());
  }

  void ProcessExpr(ExprRef ref, const StringLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    std::uint32_t string_id = AddString(expr.value);

    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetStr{
        .src_val = string_id,
        .dst_reg = reg,
    });
    expr_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const FuncCallExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    FuncCall func_call = {
        .label = syn_ctx_.DerefIdent(expr.func_name),
    };
    for (ExprRef arg : expr.args) {
      func_call.args.push_back(FuncCall::Slot{
          .reg = expr_to_reg_[arg.id()],
      });
    }
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    func_call.res = {
        .reg = reg,
    };
    am_block.instructions.push_back(std::move(func_call));
    expr_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const IdentExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    if (std::holds_alternative<ArrayType>(syn_ctx_.DerefType(expr.type)) ||
        std::holds_alternative<TupleType>(syn_ctx_.DerefType(expr.type))) {
      return;
    }
    // Read where the variable is, rather than from a copy. In single
    // assignment form a write to a variable starts a new one, so what is read
    // here never changes under the reader.
    expr_to_reg_[ref.id()] = GetVarReg(expr.name, expr.type);
  }

  void ProcessExpr(ExprRef ref, const IndexExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    LoadFromStack(ref, expr.type, am_block);
  }

  void ProcessExpr(ExprRef ref, const FieldAccessExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    LoadFromStack(ref, expr.type, am_block);
  }

  // Loads what an expression names off the stack into a register.
  //
  // An expression whose value is itself an array or a tuple is not loaded at
  // all: it has no register to go in, and what it is a step towards works out
  // where it lies for itself.
  void LoadFromStack(ExprRef ref, TypeRef type_ref,
                     AbstractMachineControlFlowGraph::Block& am_block) {
    if (IsOnStack(type_ref)) return;

    const StackPlace place = GetStackPlace(ref, am_block);
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(type_ref)};
    am_block.instructions.push_back(LoadStackReg{
        .offset = place.base_offset,
        .offset_reg = place.offset_reg.value(),
        .offset_scale = place.offset_scale,
        .dst_reg = reg,
    });
    expr_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BinaryOpExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    switch (expr.op) {
      case BinaryOp::Add:
        am_block.instructions.push_back(AddReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Sub:
        am_block.instructions.push_back(SubReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Mul:
        am_block.instructions.push_back(MulReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Div:
        am_block.instructions.push_back(DivReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Mod:
        am_block.instructions.push_back(ModReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Gt:
        am_block.instructions.push_back(GtReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Lt:
        am_block.instructions.push_back(LtReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Ge:
        am_block.instructions.push_back(GeReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Le:
        am_block.instructions.push_back(LeReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Eq:
        am_block.instructions.push_back(EqReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::NotEq:
        am_block.instructions.push_back(NotEqReg{
            .res_reg = reg,
            .lhs_reg = expr_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::And:
      case BinaryOp::Or:
        // Each of these is a branch over a variable of its own by the time
        // anything here sees the function, and so never one of these.
        assert(false && "short circuit operator reached the abstract machine");
        break;
    }
    expr_to_reg_[ref.id()] = reg;
  }

  void Process(StmtRef ref, const Stmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::visit([&](const auto& stmt) { Process(ref, stmt, am_block); }, stmt);
  }

  // What is returned is taken in by the phi function where the function ends,
  // straight from the register the value was worked out in.
  void Process(StmtRef ref, const ReturnStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    if (am_block.ref.id() >= returned_regs_.size()) {
      returned_regs_.resize(am_block.ref.id() + 1);
    }
    // What is returned goes to the phi function where the returns meet. A
    // variable given there as it is would be tied to every other value
    // returned, and spilling puts all of those in one slot while two of them
    // can be live at once, so a variable goes in a copy of its own.
    Reg value = expr_to_reg_[stmt.value.id()];
    if (std::holds_alternative<IdentExpr>(syn_ctx_.DerefExpr(stmt.value))) {
      const Reg copy = {am_cfg_.next_free_reg_id++, value.size};
      am_block.instructions.push_back(MoveReg{
          .src_reg = value,
          .dst_reg = copy,
      });
      value = copy;
    }
    returned_regs_[am_block.ref.id()] = value;
  }

  void Process(StmtRef ref, const DoStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    std::get<FuncCall>(am_block.instructions.back()).res.reset();
  }

  void Process(StmtRef ref, const VarAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto expr_type_ref = GetType(syn_ctx_.DerefExpr(stmt.expr));
    am_block.instructions.push_back(MoveReg{
        .src_reg = expr_to_reg_[stmt.expr.id()],
        .dst_reg = GetVarReg(stmt.name, expr_type_ref),
    });
  }

  void Process(StmtRef ref, const VarDeclStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    if (IsOnStack(stmt.type_constraint)) {
      var_stack_.Set(stmt.name, am_cfg_.stack_slots.size());
      PushStackSlots(stmt.type_constraint);
    } else {
      if (stmt.init.has_value()) {
        am_block.instructions.push_back(MoveReg{
            .src_reg = expr_to_reg_[stmt.init->id()],
            .dst_reg = GetVarReg(stmt.name, stmt.type_constraint),
        });
      }
    }
  }

  void Process(StmtRef ref, const ArrayAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto stmt_offset = var_stack_.Get(stmt.name);
    assert(stmt_offset.has_value());

    am_block.instructions.push_back(StoreStackReg{
        .offset = *stmt_offset,
        .offset_reg = expr_to_reg_[stmt.index.id()],
        .offset_scale = GetSize(GetType(syn_ctx_.DerefExpr(stmt.expr))),
        .src_reg = expr_to_reg_[stmt.expr.id()],
    });
  }

  void Process(StmtRef ref, const FieldAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    StackPlace place = GetStackPlace(stmt.base, am_block);
    AddToStackPlace(
        place,
        GetFieldOffset(GetType(syn_ctx_.DerefExpr(stmt.base)), stmt.field_name),
        am_block);
    am_block.instructions.push_back(StoreStackReg{
        .offset = place.base_offset,
        .offset_reg = place.offset_reg.value(),
        .offset_scale = place.offset_scale,
        .src_reg = expr_to_reg_[stmt.expr.id()],
    });
  }

  void Process(StmtRef ref, const FuncDefStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const IfStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const LoopStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  void Process(StmtRef ref, const BreakStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {}

  // Where a value lies on the stack: the slot the variable holding it starts
  // at, and how many bytes into that variable it stands, which is a register
  // times `offset_scale`. A value at the start of its variable has no
  // register.
  //
  // Indexing straight into a variable leaves the index itself as the
  // register, and the element's size as the scale, which a load or a store
  // can take as it is.
  struct StackPlace {
    std::size_t base_offset;
    std::optional<Reg> offset_reg;
    std::size_t offset_scale = 1;
  };

  // Turns the bytes `place` stands into its variable into a register of
  // their own, unless they are one already.
  void CountInBytes(StackPlace& place,
                    AbstractMachineControlFlowGraph::Block& am_block) {
    if (!place.offset_reg.has_value() || place.offset_scale == 1) return;

    Reg scale_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(SetReg{
        .src_val = static_cast<int>(place.offset_scale),
        .dst_reg = scale_reg,
    });
    Reg bytes_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(MulReg{
        .res_reg = bytes_reg,
        .lhs_reg = scale_reg,
        .rhs_reg = *place.offset_reg,
    });
    place.offset_reg = bytes_reg;
    place.offset_scale = 1;
  }

  // Adds the bytes in `step_reg` to where `place` stands.
  //
  // The sum goes in a register of its own rather than back into the one it
  // adds to, as every step does: no register is written more than once, which
  // is what lets the allocator colour registers in the order the dominator
  // tree has them in.
  void AddToStackPlace(StackPlace& place, Reg step_reg,
                       AbstractMachineControlFlowGraph::Block& am_block) {
    CountInBytes(place, am_block);
    if (!place.offset_reg.has_value()) {
      place.offset_reg = step_reg;
      return;
    }

    Reg sum_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(AddReg{
        .res_reg = sum_reg,
        .lhs_reg = *place.offset_reg,
        .rhs_reg = step_reg,
    });
    place.offset_reg = sum_reg;
  }

  // Adds `offset` bytes to where `place` stands.
  void AddToStackPlace(StackPlace& place, std::size_t offset,
                       AbstractMachineControlFlowGraph::Block& am_block) {
    Reg step_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(SetReg{
        .src_val = static_cast<int>(offset),
        .dst_reg = step_reg,
    });
    AddToStackPlace(place, step_reg, am_block);
  }

  // Works out where the value an expression names lies, by walking the
  // expression from the variable outwards and adding what each step into it
  // costs: an index costs the element's size times the index, and a field
  // costs however much of the tuple stands before it.
  StackPlace GetStackPlace(ExprRef expr_ref,
                           AbstractMachineControlFlowGraph::Block& am_block) {
    const Expr& expr = syn_ctx_.DerefExpr(expr_ref);

    if (const auto* ident_expr = std::get_if<IdentExpr>(&expr)) {
      auto pos = var_stack_.Get(ident_expr->name);
      assert(pos.has_value());
      return {.base_offset = *pos};
    }

    if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
      StackPlace place = GetStackPlace(index_expr->base, am_block);
      const Reg index_reg = expr_to_reg_[index_expr->index.id()];
      const std::size_t size = GetSize(GetType(expr));
      if (!place.offset_reg.has_value()) {
        place.offset_reg = index_reg;
        place.offset_scale = size;
        return place;
      }

      Reg size_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = static_cast<int>(size),
          .dst_reg = size_reg,
      });
      Reg step_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(MulReg{
          .res_reg = step_reg,
          .lhs_reg = size_reg,
          .rhs_reg = index_reg,
      });
      AddToStackPlace(place, step_reg, am_block);
      return place;
    }

    const auto& field_expr = std::get<FieldAccessExpr>(expr);
    StackPlace place = GetStackPlace(field_expr.base, am_block);
    AddToStackPlace(place,
                    GetFieldOffset(GetType(syn_ctx_.DerefExpr(field_expr.base)),
                                   field_expr.field_name),
                    am_block);
    return place;
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
    switch (expr_type.size) {
      case 4:
        return RegSize32;
      case 8:
        return RegSize64;
      default:
        std::unreachable();
    }
  }

  // The bytes a value of this type takes: what its elements take altogether
  // for an array, what its fields take for a tuple, and its own size for one
  // of the types the language has built in.
  std::size_t GetSize(TypeRef type_ref) {
    const Type& type = syn_ctx_.DerefType(type_ref);

    if (const auto* array_type = std::get_if<ArrayType>(&type)) {
      return static_cast<std::size_t>(array_type->size.value) *
             GetSize(array_type->element_type_constraint);
    }
    if (const auto* tuple_type = std::get_if<TupleType>(&type)) {
      std::size_t size = 0;
      for (const auto& field_ref : tuple_type->fields) {
        size += GetSize(syn_ctx_.DerefParam(field_ref).type_constraint);
      }
      return size;
    }
    return std::get<BasicType>(type).size;
  }

  // Puts a stack slot behind every value of a built in type that a value of
  // this type holds, in the order they lie one after another.
  void PushStackSlots(TypeRef type_ref) {
    const Type& type = syn_ctx_.DerefType(type_ref);

    if (const auto* array_type = std::get_if<ArrayType>(&type)) {
      for (std::int64_t i = 0; i < array_type->size.value; ++i) {
        PushStackSlots(array_type->element_type_constraint);
      }
      return;
    }
    if (const auto* tuple_type = std::get_if<TupleType>(&type)) {
      for (const auto& field_ref : tuple_type->fields) {
        PushStackSlots(syn_ctx_.DerefParam(field_ref).type_constraint);
      }
      return;
    }
    am_cfg_.stack_slots.push_back(
        static_cast<int>(std::get<BasicType>(type).size));
  }

  // How many bytes into a tuple the field called `field_name` stands.
  std::size_t GetFieldOffset(TypeRef tuple_type_ref,
                             StringIndex::Ref field_name) {
    std::size_t offset = 0;
    const auto& tuple_type =
        std::get<TupleType>(syn_ctx_.DerefType(tuple_type_ref));
    for (const auto& field_ref : tuple_type.fields) {
      const auto& field = syn_ctx_.DerefParam(field_ref);
      if (field.name == field_name) break;

      offset += GetSize(field.type_constraint);
    }
    return offset;
  }

  // What a value of this type is, where its size does not say.
  ValueKind KindOf(TypeRef type_ref) const {
    const auto* basic_type =
        std::get_if<BasicType>(&syn_ctx_.DerefType(type_ref));
    if (basic_type != nullptr &&
        syn_ctx_.DerefIdent(basic_type->name) == "String") {
      return ValueKind::String;
    }
    return ValueKind::Number;
  }

  // Whether a value of this type lies on the stack rather than in a register,
  // which is what an array or a tuple does however small it is.
  bool IsOnStack(TypeRef type_ref) {
    const Type& type = syn_ctx_.DerefType(type_ref);
    return std::holds_alternative<ArrayType>(type) ||
           std::holds_alternative<TupleType>(type);
  }

  const SyntaxContext& syn_ctx_;
  const SyntaxControlFlowGraph& syn_cfg_;
  AbstractMachineState& am_state_;
  AbstractMachineControlFlowGraph am_cfg_;

  // What the work this function asks to have done during compilation has
  // spent so far, which the interpreter counts against what compilation will
  // spend altogether.
  std::int64_t comp_steps_ = 0;

  Interpreter vm_;
  Reg result_reg_;
  // The register holding the value each block returns, by the block's ID.
  std::vector<std::optional<Reg>> returned_regs_;
  HashMap<StringIndex::Ref, Reg> var_to_reg_;
  HashMap<StringIndex::Ref, std::size_t> var_stack_;
  HashMap<SyntaxControlFlowGraph::BlockRef,
          AbstractMachineControlFlowGraph::BlockRef>
      graph_map_;
  std::vector<Reg>& expr_to_reg_;
};

}  // namespace

std::expected<AbstractMachineControlFlowGraph, CompError>
GenerateAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg,
    AbstractMachineState& am_state) {
  return AbstractMachineFunctionGenerator(am_cfgs, syn_ctx, syn_cfg, am_state)
      .Generate();
}

}  // namespace lucid
