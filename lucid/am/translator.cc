#include "lucid/am/translator.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
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
        vm_(am_cfgs, am_state, am_cfg_.stack_slots) {
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
    am_cfg_.result_kind = KindOf(syn_cfg_.func_result_type);

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
    for (const auto& succ : block.succs) {
      auto am_cfg_succ = graph_map_.Get(succ);
      assert(am_cfg_succ.has_value());
      am_block.succs.push_back(*am_cfg_succ);
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
    am_block.instructions.push_back(SetValue(expr.value, reg, am_state_));
    expr_and_stmt_to_reg_[ref.id()] = reg;
  }

  void ProcessExpr(ExprRef ref, const BoolLitExpr& expr,
                   AbstractMachineControlFlowGraph::Block& am_block) {
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(SetReg{
        .src_val = expr.value ? 1 : 0,
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
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
    if (std::holds_alternative<ArrayType>(syn_ctx_.DerefType(expr.type)) ||
        std::holds_alternative<TupleType>(syn_ctx_.DerefType(expr.type))) {
      return;
    }
    Reg reg = {am_cfg_.next_free_reg_id++, GetRegSize(expr.type)};
    am_block.instructions.push_back(MoveReg{
        .src_reg = GetVarReg(expr.name, expr.type),
        .dst_reg = reg,
    });
    expr_and_stmt_to_reg_[ref.id()] = reg;
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
        .offset_reg = place.offset_reg,
        .dst_reg = reg,
    });
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
      case BinaryOp::Ge:
        am_block.instructions.push_back(GeReg{
            .res_reg = reg,
            .lhs_reg = expr_and_stmt_to_reg_[expr.lhs.id()],
            .rhs_reg = expr_and_stmt_to_reg_[expr.rhs.id()],
        });
        break;
      case BinaryOp::Le:
        am_block.instructions.push_back(LeReg{
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
      case BinaryOp::And:
      case BinaryOp::Or:
        // Each of these is a branch over a variable of its own by the time
        // anything here sees the function, and so never one of these.
        assert(false && "short circuit operator reached the abstract machine");
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
    if (IsOnStack(stmt.type_constraint)) {
      var_stack_.Set(stmt.name, am_cfg_.stack_slots.size());
      PushStackSlots(stmt.type_constraint);
    } else {
      if (stmt.init.has_value()) {
        am_block.instructions.push_back(MoveReg{
            .src_reg = expr_and_stmt_to_reg_[stmt.init->id()],
            .dst_reg = GetVarReg(stmt.name, stmt.type_constraint),
        });
      }
    }
  }

  void Process(StmtRef ref, const ArrayAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    auto stmt_offset = var_stack_.Get(stmt.name);
    assert(stmt_offset.has_value());

    Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(SetReg{
        .src_val =
            static_cast<int>(GetSize(GetType(syn_ctx_.DerefExpr(stmt.expr)))),
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

  void Process(StmtRef ref, const FieldAssignStmt& stmt,
               AbstractMachineControlFlowGraph::Block& am_block) {
    StackPlace place = GetStackPlace(stmt.base, am_block);
    AddToStackPlace(
        place,
        GetFieldOffset(GetType(syn_ctx_.DerefExpr(stmt.base)), stmt.field_name),
        am_block);
    am_block.instructions.push_back(StoreStackReg{
        .offset = place.base_offset,
        .offset_reg = place.offset_reg,
        .src_reg = expr_and_stmt_to_reg_[stmt.expr.id()],
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
  // at, and a register carrying how many bytes into that variable it stands.
  struct StackPlace {
    std::size_t base_offset;
    Reg offset_reg;
  };

  // Adds `offset` bytes to where `place` stands.
  void AddToStackPlace(StackPlace& place, std::size_t offset,
                       AbstractMachineControlFlowGraph::Block& am_block) {
    Reg step_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
    am_block.instructions.push_back(SetReg{
        .src_val = static_cast<int>(offset),
        .dst_reg = step_reg,
    });
    am_block.instructions.push_back(AddReg{
        .res_reg = place.offset_reg,
        .lhs_reg = place.offset_reg,
        .rhs_reg = step_reg,
    });
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

      Reg offset_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = 0,
          .dst_reg = offset_reg,
      });
      return {.base_offset = *pos, .offset_reg = offset_reg};
    }

    if (const auto* index_expr = std::get_if<IndexExpr>(&expr)) {
      StackPlace place = GetStackPlace(index_expr->base, am_block);

      Reg step_reg = {am_cfg_.next_free_reg_id++, RegSize::RegSize32};
      am_block.instructions.push_back(SetReg{
          .src_val = static_cast<int>(GetSize(GetType(expr))),
          .dst_reg = step_reg,
      });
      am_block.instructions.push_back(MulReg{
          .res_reg = step_reg,
          .lhs_reg = step_reg,
          .rhs_reg = expr_and_stmt_to_reg_[index_expr->index.id()],
      });
      am_block.instructions.push_back(AddReg{
          .res_reg = place.offset_reg,
          .lhs_reg = place.offset_reg,
          .rhs_reg = step_reg,
      });
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
  Interpreter vm_;
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
