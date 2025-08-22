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
            PrintStmt(stmt_ref);
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void PrintStmt(StmtRef ref) {
    std::visit(
        [&](const auto& stmt) {
          Out() << "<S" << ref << "> ";
          Print(stmt);
        },
        ctx_.DerefStmt(ref));
  }

  void PrintExpr(ExprRef ref) {
    std::visit(
        [&](const auto& expr) {
          Out() << "<E" << ref << "> ";
          Print(expr);
        },
        ctx_.DerefExpr(ref));
  }

 private:
  void Print(const FuncParam& param) {
    Out() << "FuncParam {" << std::endl;
    Nested([&] { Out() << ".name = \"" << param.name << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const VarDeclStmt& stmt) {
    OutR() << "VarDeclStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      if (stmt.init.has_value()) {
        Out() << ".init = {" << std::endl;
        Nested([&] { PrintExpr(*stmt.init); });
        Out() << "}" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const VarAssignStmt& stmt) {
    OutR() << "VarAssignStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      Out() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const ArrayAssignStmt& stmt) {
    OutR() << "ArrayAssignStmt {" << std::endl;
    Nested([&] {
      Out() << ".name = \"" << stmt.name << "\"" << std::endl;

      Out() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(stmt.index); });
      Out() << "}" << std::endl;

      Out() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const ReturnStmt& stmt) {
    OutR() << "ReturnStmt {" << std::endl;
    Nested([&] {
      Out() << ".value = {" << std::endl;
      Nested([&] { PrintExpr(stmt.value); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const DoStmt& stmt) {
    OutR() << "DoStmt {" << std::endl;
    Nested([&] {
      Out() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const IfStmt& stmt) {
    OutR() << "IfStmt {" << std::endl;
    Nested([&] {
      Out() << ".cond = {" << std::endl;
      Nested([&] { PrintExpr(stmt.cond); });
      Out() << "}" << std::endl;

      if (stmt.then_stmts.size() > 0) {
        Out() << ".then_stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.then_stmts) {
            PrintExpr(stmt_ref);
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const LoopStmt& stmt) {
    OutR() << "LoopStmt {" << std::endl;
    Nested([&] {
      if (stmt.stmts.size() > 0) {
        Out() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt : stmt.stmts) {
            PrintStmt(stmt);
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const BreakStmt& stmt) { OutR() << "BreakStmt {}" << std::endl; }

  void Print(const FuncCallExpr& expr) {
    OutR() << "FuncCallExpr {" << std::endl;
    Nested([&] {
      Out() << ".func_name = \"" << expr.func_name << "\"" << std::endl;

      if (expr.args.size() > 0) {
        Out() << ".args = [" << std::endl;
        Nested([&] {
          for (ExprRef arg : expr.args) {
            PrintExpr(arg);
          }
        });
        Out() << "]" << std::endl;
      }
    });
    Out() << "}" << std::endl;
  }

  void Print(const IntLitExpr& expr) {
    OutR() << "IntLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = " << expr.value << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const BoolLitExpr& expr) {
    OutR() << "BoolLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = " << expr.value << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const StringLitExpr& expr) {
    OutR() << "StringLitExpr {" << std::endl;
    Nested([&] { Out() << ".value = \"" << expr.value << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const IdentExpr& expr) {
    OutR() << "IdentExpr {" << std::endl;
    Nested([&] { Out() << ".name = \"" << expr.name << "\"" << std::endl; });
    Out() << "}" << std::endl;
  }

  void Print(const IndexExpr& expr) {
    OutR() << "IndexExpr {" << std::endl;
    Nested([&] {
      Out() << ".base = {" << std::endl;
      Nested([&] { PrintExpr(expr.base); });
      Out() << "}" << std::endl;

      Out() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(expr.index); });
      Out() << "}" << std::endl;
    });
    Out() << "}" << std::endl;
  }

  void Print(const BinaryOpExpr& expr) {
    OutR() << "BinaryOpExpr {" << std::endl;
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
      Nested([&] { PrintExpr(expr.lhs); });
      Out() << "}" << std::endl;

      Out() << ".rhs = {" << std::endl;
      Nested([&] { PrintExpr(expr.rhs); });
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

  std::ostream& OutR() { return std::cout; }

  const SyntaxContext ctx_;
  int indent_ = 0;
};

}  // namespace

void Print(const SyntaxContext ctx, const FuncDefStmt& stmt) {
  AstPrinter(ctx).Print(stmt);
}

void PrintStmt(const SyntaxContext ctx, StmtRef ref) {
  AstPrinter(ctx).PrintStmt(ref);
}

void PrintExpr(const SyntaxContext ctx, ExprRef ref) {
  AstPrinter(ctx).PrintExpr(ref);
}

}  // namespace lucid
