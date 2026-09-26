#include "lucid/syntax/scope.h"

#include <expected>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// The names the variables of a function go by as it is walked, and what to
// put back as the walk leaves the block that gave them those names.
class Scope {
 public:
  // Records that `from` goes by `to` from here on.
  void Define(StringIndex::Ref from, StringIndex::Ref to) {
    const auto in_force = names_.Get(from);
    undo_.emplace_back(from, in_force.has_value()
                                 ? std::optional<StringIndex::Ref>(*in_force)
                                 : std::nullopt);
    names_.Set(from, to);
  }

  // Returns the name `from` goes by, or nothing where no declaration of it
  // stands over here.
  std::optional<StringIndex::Ref> InForce(StringIndex::Ref from) const {
    const auto to = names_.Get(from);
    if (!to.has_value()) return std::nullopt;
    return *to;
  }

  // Returns a mark that `UndoTo` takes the names back to.
  std::size_t Mark() const { return undo_.size(); }

  // Takes back every definition made since `mark`, which is what leaving a
  // block does to the declarations made inside it.
  void UndoTo(std::size_t mark) {
    while (undo_.size() > mark) {
      const auto& [from, in_force] = undo_.back();
      if (in_force.has_value()) {
        names_.Set(from, *in_force);
      } else {
        names_.Remove(from);
      }
      undo_.pop_back();
    }
  }

 private:
  HashMap<StringIndex::Ref, StringIndex::Ref> names_;
  std::vector<std::pair<StringIndex::Ref, std::optional<StringIndex::Ref>>>
      undo_;
};

class NameResolver {
 public:
  NameResolver(SyntaxContext& syn_ctx, FuncDefStmt& func_def)
      : syn_ctx_(syn_ctx), func_def_(func_def) {}

  std::expected<void, ScopeError> Resolve() {
    // A parameter is declared where the function is, so it stands over the
    // whole body and keeps the name it was written with.
    for (const auto& param_ref : func_def_.params) {
      const auto& param = syn_ctx_.DerefParam(param_ref);
      scope_.Define(param.name, param.name);
    }

    ResolveBlock(func_def_.stmts);

    if (error_.has_value()) return std::unexpected(ScopeError(*error_));
    return {};
  }

 private:
  void Fail(std::string message) {
    if (!error_.has_value()) error_ = std::move(message);
  }

  std::string NameOf(StringIndex::Ref name) {
    return std::string(syn_ctx_.DerefIdent(name));
  }

  // Rewrites `name` to the declaration standing over it.
  void ResolveUse(StringIndex::Ref& name) {
    const auto in_force = scope_.InForce(name);
    if (!in_force.has_value()) {
      Fail("no variable '" + NameOf(name) + "'");
      return;
    }
    name = *in_force;
  }

  void ResolveBlock(SuccessiveList<StmtRef> stmts) {
    const std::size_t mark = scope_.Mark();
    for (StmtRef stmt_ref : stmts) ResolveStmt(stmt_ref);
    scope_.UndoTo(mark);
  }

  void ResolveStmt(StmtRef stmt_ref) {
    Stmt& stmt = syn_ctx_.DerefStmt(stmt_ref);

    if (auto* var_decl_stmt = std::get_if<VarDeclStmt>(&stmt)) {
      // The initializer stands before the declaration, so a name in it is
      // whatever it was before: `val x: Int32 = x` reads the earlier `x`.
      if (var_decl_stmt->init.has_value()) ResolveExpr(*var_decl_stmt->init);

      const auto new_name = syn_ctx_.AddUniqueIdent();
      scope_.Define(var_decl_stmt->name, new_name);
      var_decl_stmt->name = new_name;
    } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
      ResolveExpr(var_assign_stmt->expr);
      ResolveUse(var_assign_stmt->name);
    } else if (auto* array_assign_stmt = std::get_if<ArrayAssignStmt>(&stmt)) {
      ResolveExpr(array_assign_stmt->index);
      ResolveExpr(array_assign_stmt->expr);
      ResolveUse(array_assign_stmt->name);
    } else if (auto* field_assign_stmt = std::get_if<FieldAssignStmt>(&stmt)) {
      ResolveExpr(field_assign_stmt->base);
      ResolveExpr(field_assign_stmt->expr);
    } else if (auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
      ResolveExpr(return_stmt->value);
    } else if (auto* do_stmt = std::get_if<DoStmt>(&stmt)) {
      ResolveExpr(do_stmt->expr);
    } else if (auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      ResolveExpr(if_stmt->cond);
      ResolveBlock(if_stmt->then_stmts);
      ResolveBlock(if_stmt->else_stmts);
    } else if (auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
      ResolveBlock(loop_stmt->stmts);
    }
  }

  void ResolveExpr(ExprRef expr_ref) {
    Expr& expr = syn_ctx_.DerefExpr(expr_ref);

    if (auto* ident_expr = std::get_if<IdentExpr>(&expr)) {
      ResolveUse(ident_expr->name);
    } else if (auto* func_call_expr = std::get_if<FuncCallExpr>(&expr)) {
      for (ExprRef arg : func_call_expr->args) ResolveExpr(arg);
    } else if (auto* index_expr = std::get_if<IndexExpr>(&expr)) {
      ResolveExpr(index_expr->base);
      ResolveExpr(index_expr->index);
    } else if (auto* field_access_expr = std::get_if<FieldAccessExpr>(&expr)) {
      ResolveExpr(field_access_expr->base);
    } else if (auto* binary_op_expr = std::get_if<BinaryOpExpr>(&expr)) {
      ResolveExpr(binary_op_expr->lhs);
      ResolveExpr(binary_op_expr->rhs);
    }
  }

  SyntaxContext& syn_ctx_;
  FuncDefStmt& func_def_;
  Scope scope_;
  std::optional<std::string> error_;
};

}  // namespace

std::expected<void, ScopeError> ResolveNames(SyntaxContext& syn_ctx,
                                             FuncDefStmt& stmt) {
  return NameResolver(syn_ctx, stmt).Resolve();
}

}  // namespace lucid
