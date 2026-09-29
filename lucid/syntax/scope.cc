#include "lucid/syntax/scope.h"

#include <expected>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
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
      if (param.is_mutable) MarkWritable(param.name);
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

  // Records that what `name` now goes by may be written.
  void MarkWritable(StringIndex::Ref name) { writable_.Insert(name); }

  // Reports a write to a name whose declaration does not allow one.
  //
  // The `mut` on a declaration is what says a write to it can happen, so one
  // without it is written once where it is made and read from then on.
  void CheckWritable(StringIndex::Ref name, StringIndex::Ref written_as) {
    if (writable_.Contains(name)) return;

    Fail("no 'mut' on the declaration of '" + NameOf(written_as) + "'");
  }

  // Returns the name at the foot of an assignment's target, which is the
  // variable a write through it reaches. A target that does not stand on one
  // is left to whatever else has something to say about it.
  std::optional<StringIndex::Ref> RootOf(ExprRef expr_ref) {
    const Expr* expr = &syn_ctx_.DerefExpr(expr_ref);
    while (true) {
      if (const auto* ident = std::get_if<IdentExpr>(expr)) {
        return ident->name;
      }
      if (const auto* index = std::get_if<IndexExpr>(expr)) {
        expr = &syn_ctx_.DerefExpr(index->base);
        continue;
      }
      if (const auto* field = std::get_if<FieldAccessExpr>(expr)) {
        expr = &syn_ctx_.DerefExpr(field->base);
        continue;
      }
      return std::nullopt;
    }
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
      if (var_decl_stmt->is_mutable) MarkWritable(new_name);
      var_decl_stmt->name = new_name;
    } else if (auto* var_assign_stmt = std::get_if<VarAssignStmt>(&stmt)) {
      ResolveExpr(var_assign_stmt->expr);
      const auto written_as = var_assign_stmt->name;
      ResolveUse(var_assign_stmt->name);
      CheckWritable(var_assign_stmt->name, written_as);
    } else if (auto* array_assign_stmt = std::get_if<ArrayAssignStmt>(&stmt)) {
      ResolveExpr(array_assign_stmt->index);
      ResolveExpr(array_assign_stmt->expr);
      const auto written_as = array_assign_stmt->name;
      ResolveUse(array_assign_stmt->name);
      CheckWritable(array_assign_stmt->name, written_as);
    } else if (auto* field_assign_stmt = std::get_if<FieldAssignStmt>(&stmt)) {
      // Read before the walk rewrites it, so that what is reported back is
      // the name as it was written rather than the one it now goes by.
      const auto written_as = RootOf(field_assign_stmt->base);

      ResolveExpr(field_assign_stmt->base);
      ResolveExpr(field_assign_stmt->expr);

      // The write reaches through the target to the variable at its foot,
      // which is the one whose declaration has to allow it.
      if (const auto root = RootOf(field_assign_stmt->base); root.has_value()) {
        CheckWritable(*root, written_as.value_or(*root));
      }
    } else if (auto* return_stmt = std::get_if<ReturnStmt>(&stmt)) {
      ResolveExpr(return_stmt->value);
    } else if (auto* do_stmt = std::get_if<DoStmt>(&stmt)) {
      ResolveExpr(do_stmt->expr);
    } else if (auto* if_stmt = std::get_if<IfStmt>(&stmt)) {
      ResolveExpr(if_stmt->cond);
      ResolveBlock(if_stmt->then_stmts);
      ResolveBlock(if_stmt->else_stmts);
    } else if (auto* loop_stmt = std::get_if<LoopStmt>(&stmt)) {
      ++loops_;
      ResolveBlock(loop_stmt->stmts);
      --loops_;
    } else if (std::holds_alternative<BreakStmt>(stmt)) {
      // A `break` leaves the innermost loop standing over it, and one with
      // no loop over it at all has nowhere to go.
      if (loops_ == 0) Fail("break with no loop to leave");
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

  // The names a write can reach, by what each goes by after resolving. A
  // block that ends does not take its names out of here: they are unique to
  // the declaration that made them, so nothing later goes by one of them.
  HashSet<StringIndex::Ref> writable_;

  // How many loops the walk stands inside, which is what says whether a
  // `break` has one to leave.
  int loops_ = 0;

  std::optional<std::string> error_;
};

}  // namespace

std::expected<void, ScopeError> ResolveNames(SyntaxContext& syn_ctx,
                                             FuncDefStmt& stmt) {
  return NameResolver(syn_ctx, stmt).Resolve();
}

}  // namespace lucid
