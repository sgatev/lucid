#include "lucid/static.h"

#include <variant>

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/result.h"

namespace lucid {
namespace {

class StaticExprInferenceEngine {
 public:
  StaticExprInferenceEngine(SyntaxContext& ctx, ControlFlowGraph& cfg)
      : ctx_(ctx), cfg_(cfg) {}

  Result<void, StaticError> Infer() {
    for (auto& block : cfg_.blocks()) {
      for (auto& seq : block.sequences) {
        for (auto& expr : seq.expressions) {
          ProcessExpr(ctx_.DerefExpr(expr));
        }
      }
    }
    return {};
  }

 private:
  void ProcessExpr(Expr& expr) {
    std::visit([](auto& expr) { expr.is_static = false; }, expr);

    if (auto* _ = std::get_if<FuncCallExpr>(&expr)) {
    } else if (auto* bool_lit_expr = std::get_if<BoolLitExpr>(&expr)) {
      bool_lit_expr->is_static = true;
    } else if (auto* int_lit_expr = std::get_if<IntLitExpr>(&expr)) {
      int_lit_expr->is_static = true;
    } else if (auto* _ = std::get_if<IdentExpr>(&expr)) {
    } else if (auto* _ = std::get_if<IndexExpr>(&expr)) {
    } else if (auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
      binary_op_expr->is_static =
          IsStatic(binary_op_expr->lhs) && IsStatic(binary_op_expr->rhs);
    }
  }

  bool IsStatic(ExprRef ref) const {
    return std::visit([](auto& expr) { return expr.is_static; },
                      ctx_.DerefExpr(ref));
  }

  SyntaxContext& ctx_;
  ControlFlowGraph& cfg_;
};

}  // namespace

Result<void, StaticError> InferStaticExprs(SyntaxContext& ctx,
                                           ControlFlowGraph& cfg) {
  return StaticExprInferenceEngine(ctx, cfg).Infer();
}

}  // namespace lucid
