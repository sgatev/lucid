#include "lucid/syntax/cfg.h"

#include <stack>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/container/successive_list.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Builds the control flow graph of a function.
class ControlFlowGraphBuilder {
  using BlockRef = SyntaxControlFlowGraph::BlockRef;
  using Sequence = SyntaxControlFlowGraph::Sequence;

 public:
  ControlFlowGraphBuilder(SyntaxContext& ctx, const FuncDefStmt& func_def)
      : ctx_(ctx),
        bool_type_(ctx.ResolveType(ctx.AddIdent("Bool"))),
        graph_(func_def.name) {
    graph_.func_params = func_def.params;
    graph_.func_result_type = func_def.result_type;
    graph_.first = AddBlock();
    graph_.last = AddBlock();
    BuildBlock(func_def.stmts, graph_.first, graph_.last);
  }

  // Returns the constructed control flow graph.
  SyntaxControlFlowGraph Consume() && { return std::move(graph_); }

 private:
  BlockRef AddBlock() {
    SyntaxControlFlowGraph::Block block;
    auto ref = graph_.add(std::move(block));
    graph_.get(ref).ref = ref;
    return ref;
  }

  bool BuildBlock(const SuccessiveList<StmtRef>& stmts, BlockRef block,
                  BlockRef end) {
    bool should_connect = true;
    for (StmtRef stmt_ref : stmts) {
      if (auto* loop_stmt = std::get_if<LoopStmt>(&ctx_.DerefStmt(stmt_ref))) {
        auto post_loop_block = AddBlock();

        post_loop_blocks_.push(post_loop_block);

        auto loop_block = AddBlock();
        BuildBlock(loop_stmt->stmts, loop_block, loop_block);
        graph_.get(block).succs.push_back(loop_block);
        graph_.get(loop_block).preds.push_back(block);

        post_loop_blocks_.pop();

        // The sequence carries on in post_loop_block, which is where a `break`
        // jumps to, so it still connects to `end`. BuildBlock's result provided
        // an answer for the loop's body, not for this sequence.
        should_connect = true;

        block = post_loop_block;
      } else if (auto* if_stmt =
                     std::get_if<IfStmt>(&ctx_.DerefStmt(stmt_ref))) {
        const ExprRef cond = std::get<IfStmt>(ctx_.DerefStmt(stmt_ref)).cond;

        std::vector<ExprRef> cond_exprs;
        EmitExpr(cond, cond_exprs, block);
        graph_.get(block).sequences.emplace_back().expressions =
            std::move(cond_exprs);

        if_stmt = &std::get<IfStmt>(ctx_.DerefStmt(stmt_ref));
        auto post_if_block = AddBlock();

        auto then_block = AddBlock();
        bool then_continues =
            BuildBlock(if_stmt->then_stmts, then_block, post_if_block);
        graph_.get(block).succs.push_back(then_block);
        graph_.get(then_block).preds.push_back(block);

        if (if_stmt->else_stmts.size() == 0) {
          graph_.get(block).succs.push_back(post_if_block);
          graph_.get(post_if_block).preds.push_back(block);
        } else {
          auto else_block = AddBlock();
          bool else_continues =
              BuildBlock(if_stmt->else_stmts, else_block, post_if_block);
          graph_.get(block).succs.push_back(else_block);
          graph_.get(else_block).preds.push_back(block);

          should_connect = then_continues || else_continues;
        }

        graph_.get(block).branch_cond = cond;
        block = post_if_block;
      } else {
        std::vector<ExprRef> exprs;
        EmitStmtExprs(stmt_ref, exprs, block);

        auto& seq = graph_.get(block).sequences.emplace_back();
        seq.stmt = stmt_ref;
        seq.expressions = std::move(exprs);

        if (std::holds_alternative<ReturnStmt>(ctx_.DerefStmt(stmt_ref))) {
          graph_.get(block).succs.push_back(graph_.last);
          graph_.get(graph_.last).preds.push_back(block);
          return false;
        }

        if (std::holds_alternative<BreakStmt>(ctx_.DerefStmt(stmt_ref))) {
          graph_.get(block).succs.push_back(post_loop_blocks_.top());
          graph_.get(post_loop_blocks_.top()).preds.push_back(block);
          return false;
        }
      }
    }

    if (should_connect) {
      graph_.get(block).succs.push_back(end);
      graph_.get(end).preds.push_back(block);
    }

    return true;
  }

  // Reads the expressions a statement is made of into the block the walk
  // stands in, in the order they are read.
  void EmitStmtExprs(StmtRef stmt_ref, std::vector<ExprRef>& exprs,
                     BlockRef& block) {
    const Stmt stmt = ctx_.DerefStmt(stmt_ref);

    if (const auto* s = std::get_if<ReturnStmt>(&stmt)) {
      EmitExpr(s->value, exprs, block);
    } else if (const auto* s = std::get_if<DoStmt>(&stmt)) {
      EmitExpr(s->expr, exprs, block);
    } else if (const auto* s = std::get_if<VarDeclStmt>(&stmt)) {
      if (s->init.has_value()) EmitExpr(*s->init, exprs, block);
    } else if (const auto* s = std::get_if<VarAssignStmt>(&stmt)) {
      EmitExpr(s->expr, exprs, block);
    } else if (const auto* s = std::get_if<ArrayAssignStmt>(&stmt)) {
      EmitExpr(s->index, exprs, block);
      EmitExpr(s->expr, exprs, block);
    } else if (const auto* s = std::get_if<FieldAssignStmt>(&stmt)) {
      EmitExpr(s->base, exprs, block);
      EmitExpr(s->expr, exprs, block);
    }
  }

  // Reads an expression into the block the walk stands in, adding what it is
  // made of to `exprs` in the order it is read: what an operation is over
  // before the operation itself.
  //
  // Reading `and` or `or` takes a branch, so the walk can come out of this in
  // a later block than it went into it. What was read before the branch is
  // left in the block it was read in.
  void EmitExpr(ExprRef expr_ref, std::vector<ExprRef>& exprs,
                BlockRef& block) {
    const Expr expr = ctx_.DerefExpr(expr_ref);

    if (const auto* e = std::get_if<BinaryOpExpr>(&expr)) {
      if (e->op == BinaryOp::And || e->op == BinaryOp::Or) {
        // Branching is how the right side is left unread, and the only
        // reason to leave it unread is what reading it would do. Work done
        // during compilation does nothing: it calls comp functions alone,
        // and none of those holds a `do`. So there it reads both sides and
        // stays the one expression, which is what lets its value be worked
        // out where every other comp value is.
        if (e->is_comp) {
          EmitCompShortCircuit(expr_ref, *e, exprs, block);
        } else {
          EmitShortCircuit(expr_ref, *e, exprs, block);
        }
        return;
      }
      EmitExpr(e->lhs, exprs, block);
      EmitExpr(e->rhs, exprs, block);
    } else if (const auto* e = std::get_if<FuncCallExpr>(&expr)) {
      graph_.has_func_calls = true;
      for (ExprRef arg : e->args) EmitExpr(arg, exprs, block);
    } else if (const auto* e = std::get_if<IndexExpr>(&expr)) {
      EmitExpr(e->base, exprs, block);
      EmitExpr(e->index, exprs, block);
    } else if (const auto* e = std::get_if<FieldAccessExpr>(&expr)) {
      EmitExpr(e->base, exprs, block);
    }

    exprs.push_back(expr_ref);
  }

  // Reads both sides of `and` or `or` and puts what they come to in place
  // of the operator, for the one case where reading both is the same as
  // reading one: work done during compilation.
  //
  // A `Bool` is a zero or a one, so both sides hold exactly where the two
  // multiply to one, and either holds exactly where they add to something
  // other than zero.
  void EmitCompShortCircuit(ExprRef expr_ref, const BinaryOpExpr& expr,
                            std::vector<ExprRef>& exprs, BlockRef& block) {
    EmitExpr(expr.lhs, exprs, block);
    EmitExpr(expr.rhs, exprs, block);

    if (expr.op == BinaryOp::And) {
      ctx_.DerefExpr(expr_ref) =
          CompBoolExpr(BinaryOp::Mul, expr.lhs, expr.rhs);
      exprs.push_back(expr_ref);
      return;
    }

    BoolLitExpr zero{.value = false};
    zero.type = bool_type_;
    zero.is_comp = true;
    const ExprRef zero_ref = ctx_.Add(zero);
    const ExprRef sum_ref =
        ctx_.Add(CompBoolExpr(BinaryOp::Add, expr.lhs, expr.rhs));

    ctx_.DerefExpr(expr_ref) = CompBoolExpr(BinaryOp::NotEq, sum_ref, zero_ref);

    exprs.push_back(sum_ref);
    exprs.push_back(zero_ref);
    exprs.push_back(expr_ref);
  }

  BinaryOpExpr CompBoolExpr(BinaryOp op, ExprRef lhs, ExprRef rhs) {
    BinaryOpExpr expr{.op = op, .lhs = lhs, .rhs = rhs};
    expr.type = bool_type_;
    expr.is_comp = true;
    return expr;
  }

  // Turns `and` or `or` into a branch over a variable of its own.
  //
  // The value comes out of one of two blocks, and what a graph can bring
  // back together where blocks meet is a variable: that is what a phi
  // function is made for. So the operator becomes a variable holding its
  // left side, a branch writing its right side into that variable where the
  // left side leaves the answer open, and a read of the variable after.
  void EmitShortCircuit(ExprRef expr_ref, const BinaryOpExpr& expr,
                        std::vector<ExprRef>& exprs, BlockRef& block) {
    EmitExpr(expr.lhs, exprs, block);

    const StringIndex::Ref name = ctx_.AddUniqueIdent();
    const StmtRef decl = ctx_.Add(VarDeclStmt{
        .name = name,
        .type_constraint = bool_type_,
        .init = expr.lhs,
    });
    {
      auto& seq = graph_.get(block).sequences.emplace_back();
      seq.expressions = std::move(exprs);
      seq.stmt = decl;
    }
    exprs.clear();
    graph_.get(block).branch_cond = expr.lhs;

    const BlockRef rhs_block = AddBlock();
    const BlockRef join_block = AddBlock();

    // A conjunction reads its right side where its left side holds, and a
    // disjunction where its left side does not.
    const bool is_and = expr.op == BinaryOp::And;
    for (BlockRef succ :
         {is_and ? rhs_block : join_block, is_and ? join_block : rhs_block}) {
      graph_.get(block).succs.push_back(succ);
      graph_.get(succ).preds.push_back(block);
    }

    BlockRef rhs_end = rhs_block;
    std::vector<ExprRef> rhs_exprs;
    EmitExpr(expr.rhs, rhs_exprs, rhs_end);

    const StmtRef assign =
        ctx_.Add(VarAssignStmt{.name = name, .expr = expr.rhs});
    {
      auto& seq = graph_.get(rhs_end).sequences.emplace_back();
      seq.expressions = std::move(rhs_exprs);
      seq.stmt = assign;
    }
    graph_.get(rhs_end).succs.push_back(join_block);
    graph_.get(join_block).preds.push_back(rhs_end);

    block = join_block;

    // What stood here reads the variable now.
    IdentExpr ident{.name = name};
    ident.type = bool_type_;
    ctx_.DerefExpr(expr_ref) = ident;
    exprs.push_back(expr_ref);
  }

  SyntaxContext& ctx_;
  TypeRef bool_type_;
  SyntaxControlFlowGraph graph_;
  std::stack<BlockRef> post_loop_blocks_;
};

}  // namespace

SyntaxControlFlowGraph BuildControlFlowGraph(SyntaxContext& ctx,
                                             const FuncDefStmt& func) {
  return ControlFlowGraphBuilder(ctx, func).Consume();
}

}  // namespace lucid
