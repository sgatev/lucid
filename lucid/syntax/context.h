#pragma once

#include <cstddef>
#include <list>
#include <string>
#include <string_view>

#include "lucid/core/container/arena.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/type_repository.h"

namespace lucid {

// A context for syntactic operations.
class SyntaxContext {
 public:
  SyntaxContext() : types_(idents_) {}

  // Makes room for the nodes of a source of `size` characters.
  void ReserveForSource(std::size_t size) {
    stmts_.Reserve(size / kCharsPerStmt);
    exprs_.Reserve(size / kCharsPerExpr);
  }

  // Adds `stmt` to the context.
  StmtRef Add(Stmt stmt) { return stmts_.Add(stmt); }

  // Adds `expr` to the context.
  ExprRef Add(Expr expr) { return exprs_.Add(expr); }

  // Adds `type` to the context.
  TypeRef Add(Type type) { return types_.Add(type); }

  // Adds `param` to the context.
  ParamRef Add(FuncParam param) { return params_.Add(param); }

  // Adds `ident` to the context.
  StringIndex::Ref AddIdent(std::string_view ident) {
    return idents_.ref(ident);
  }

  // Adds a unique ident to the context.
  StringIndex::Ref AddUniqueIdent() {
    unique_idents_.push_back("$" + std::to_string(unique_idents_.size()));
    return idents_.ref(unique_idents_.back());
  }

  // Creates an alias of `ref` in the context.
  StmtRef AliasStmt(StmtRef ref) { return stmts_.Alias(ref); }

  // Creates an alias of `ref` in the context.
  ExprRef AliasExpr(ExprRef ref) { return exprs_.Alias(ref); }

  // Creates an alias of `ref` in the context.
  ParamRef AliasParam(ParamRef ref) { return params_.Alias(ref); }

  // Returns the statement that `ref` refers to.
  auto& DerefStmt(this auto&& self, StmtRef ref) {
    return self.stmts_.Get(ref);
  }

  // Returns true iff `ref` refers to a statement in the context.
  bool ContainsStmt(StmtRef ref) const { return stmts_.Contains(ref); }

  // Returns the expression that `ref` refers to.
  auto& DerefExpr(this auto&& self, ExprRef ref) {
    return self.exprs_.Get(ref);
  }

  // Returns true iff `ref` refers to an expression in the context.
  bool ContainsExpr(ExprRef ref) const { return exprs_.Contains(ref); }

  // Returns a reference to the type that `name` resolves to.
  TypeRef ResolveType(StringIndex::Ref name) const {
    return types_.Resolve(name);
  }

  // Returns the type that `ref` refers to.
  const Type& DerefType(TypeRef ref) const { return types_.Deref(ref); }

  // Returns the parameter that `ref` refers to.
  auto& DerefParam(this auto&& self, ParamRef ref) {
    return self.params_.Get(ref);
  }

  // Returns the identifier that `ref` refers to.
  std::string_view DerefIdent(StringIndex::Ref ref) const {
    return idents_.deref(ref);
  }

  // Returns true if and only if `lhs` and `rhs` refer to equivalent statements.
  bool EquivStmts(StmtRef lhs, StmtRef rhs) const {
    return stmts_.Equiv(lhs, rhs);
  }

  // Returns true if and only if `lhs` and `rhs` refer to equivalent statements.
  bool Equiv(StmtRef lhs, StmtRef rhs) const { return stmts_.Equiv(lhs, rhs); }

  // Returns true if and only if `lhs` and `rhs` refer to equivalent function
  // parameters.
  bool Equiv(ParamRef lhs, ParamRef rhs) const {
    return params_.Equiv(lhs, rhs);
  }

  // Adds a function definition.
  void AddFuncDef(const FuncDefStmt& func_def) {
    func_defs_.Set(func_def.name, &func_def);
  }

  // Returns the definition of the function with the given name.
  const FuncDefStmt& GetFuncDef(StringIndex::Ref name) const {
    return **func_defs_.Get(name);
  }

  // Registers a type with the given name.
  void RegisterType(StringIndex::Ref name, TypeRef ref) {
    types_.Register(name, ref);
  }

  std::size_t Size() const {
    return stmts_.Size() + exprs_.Size() + types_.Size();
  }

 private:
  // Measured over the example programs, which run from 23 to 40 characters
  // per statement and from 9 to 30 per expression. Reserving for the dense
  // end of that would waste most of it on ordinary code, so these sit nearer
  // the middle: a source denser than this still grows, just rarely.
  static constexpr std::size_t kCharsPerStmt = 25;
  static constexpr std::size_t kCharsPerExpr = 12;

  Arena<Stmt> stmts_;
  Arena<Expr> exprs_;
  Arena<FuncParam> params_;
  std::list<std::string> unique_idents_;
  StringIndex idents_;
  TypeRepository types_;
  HashMap<StringIndex::Ref, const FuncDefStmt*> func_defs_;
};

}  // namespace lucid
