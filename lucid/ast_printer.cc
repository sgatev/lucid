#include "lucid/ast_printer.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <variant>

#include "lucid/ast.h"

namespace lucid {
namespace {

class AstPrinter {
 public:
  explicit AstPrinter(const SyntaxContext& ctx) : ctx_(ctx) {}

  void Print(const FuncDefStmt& stmt) {
    Out() << "FuncDefStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      if (stmt.params.size() > 0) {
        Out() << ".params = [" << std::endl;
        Nested([&] {
          for (ParamRef param_ref : stmt.params) {
            Print(ctx_.DerefParam(param_ref));
          }
        });
        Out() << "]" << std::endl;
      }

      if (stmt.stmts.size() > 0) {
        Out() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.stmts) {
            Print(ctx_.DerefStmt(stmt_ref));
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

 private:
  void Print(const FuncParam& param) {
    Out() << "FuncParam {" << std::endl;
    Nested([&] { Out() << ".name = \"" << param.name << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const Stmt& stmt) {
    std::visit([this](const auto& stmt) { Print(stmt); }, stmt);
  }

  void Print(const VarDeclStmt& stmt) {
    Out() << "VarDeclStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      if (stmt.init.has_value()) {
        Out() << ".init = {" << std::endl;
        Nested([&] { Print(ctx_.DerefExpr(*stmt.init)); });
        Out() << "}" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const VarAssignStmt& stmt) {
    Out() << "VarAssignStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      Out() << ".expr = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.expr)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const ArrayAssignStmt& stmt) {
    Out() << "ArrayAssignStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      Out() << ".index = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.index)); });
      Out() << "}" << std::endl;

      Out() << ".expr = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.expr)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const ReturnStmt& stmt) {
    Out() << "ReturnStmt {" << std::endl;
    Nested([&] {
      Out() << ".value = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.value)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const DoStmt& stmt) {
    Out() << "DoStmt {" << std::endl;
    Nested([&] {
      Out() << ".expr = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.expr)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const IfStmt& stmt) {
    Out() << "IfStmt {" << std::endl;
    Nested([&] {
      Out() << ".cond = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(stmt.cond)); });
      Out() << "}" << std::endl;

      if (stmt.then_stmts.size() > 0) {
        Out() << ".then_stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.then_stmts) {
            Print(ctx_.DerefStmt(stmt_ref));
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const LoopStmt& stmt) {
    Out() << "LoopStmt {" << std::endl;
    Nested([&] {
      if (stmt.stmts.size() > 0) {
        Out() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt : stmt.stmts) {
            Print(ctx_.DerefStmt(stmt));
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const BreakStmt& stmt) { Out() << "BreakStmt {}" << std::endl; }

  void Print(const Expr& expr) {
    std::visit([this](const auto& expr) { Print(expr); }, expr);
  }

  void Print(const FuncCallExpr& expr) {
    Out() << "FuncCallExpr {" << std::endl;
    Nested([&] {
      Out() << ".func_name = \"" << expr.func_name << "\"" << std::endl;

      if (expr.args.size() > 0) {
        Out() << ".args = [" << std::endl;
        Nested([&] {
          for (ExprRef arg : expr.args) {
            Print(ctx_.DerefExpr(arg));
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const IntLitExpr& expr) {
    Out() << "IntLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = " << expr.value << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const BoolLitExpr& expr) {
    Out() << "BoolLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = " << expr.value << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const StringLitExpr& expr) {
    Out() << "StringLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = \"" << expr.value << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const IdentExpr& expr) {
    Out() << "IdentExpr {" << std::endl;
    Nested([&] { Out() << ".name = \"" << expr.name << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const IndexExpr& expr) {
    Out() << "IndexExpr {" << std::endl;
    Nested([&] {
      Out() << ".base = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(expr.base)); });
      Out() << "}" << std::endl;

      Out() << ".index = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(expr.index)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const BinaryOpExpr& expr) {
    Out() << "BinaryOpExpr {" << std::endl;
    Nested([&] {
      switch (expr.op) {
        case BinaryOp::Add:
          Out() << ".op = Add" << std::endl;
          break;
        case BinaryOp::Sub:
          Out() << ".op = Sub" << std::endl;
          break;
        case BinaryOp::Mul:
          Out() << ".op = Mul" << std::endl;
          break;
        case BinaryOp::Div:
          Out() << ".op = Div" << std::endl;
          break;
        case BinaryOp::Mod:
          Out() << ".op = Mod" << std::endl;
          break;
        case BinaryOp::Gt:
          Out() << ".op = Gt" << std::endl;
          break;
        case BinaryOp::Lt:
          Out() << ".op = Lt" << std::endl;
          break;
        case BinaryOp::Eq:
          Out() << ".op = Eq" << std::endl;
          break;
        case BinaryOp::NotEq:
          Out() << ".op = NotEq" << std::endl;
          break;
      }

      Out() << ".lhs = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(expr.lhs)); });
      Out() << "}" << std::endl;

      Out() << ".rhs = {" << std::endl;
      Nested([&] { Print(ctx_.DerefExpr(expr.rhs)); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Nested(std::function<void()> f) {
    indent_ += 2;
    std::invoke(f);
    indent_ -= 2;
  }

  std::ostream& Out() {
    for (int i = 0; i < indent_; ++i) std::cout << " ";
    return std::cout;
  }

  const SyntaxContext ctx_;
  int indent_ = 0;
};

}  // namespace

void Print(const SyntaxContext ctx, const FuncDefStmt& stmt) {
  AstPrinter(ctx).Print(stmt);
}

}  // namespace lucid
