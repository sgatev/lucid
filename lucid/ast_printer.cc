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
    OutFmt() << "FuncDefStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".name = \"" << stmt.name << "\"" << std::endl;

      if (stmt.params.size() > 0) {
        OutFmt() << ".params = [" << std::endl;
        Nested([&] {
          for (ParamRef param_ref : stmt.params) {
            Print(ctx_.DerefParam(param_ref));
          }
        });
        OutFmt() << "]" << std::endl;
      }

      if (stmt.stmts.size() > 0) {
        OutFmt() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.stmts) {
            PrintStmt(stmt_ref);
          }
        });
        OutFmt() << "]" << std::endl;
      }
    });
    OutFmt() << "}" << std::endl;
  }

  void PrintStmt(StmtRef ref) {
    std::visit(
        [&](const auto& stmt) {
          Blue([&] { OutFmt() << "S" << ref << ":"; });
          Print(stmt);
        },
        ctx_.DerefStmt(ref));
  }

  void PrintExpr(ExprRef ref) {
    std::visit(
        [&](const auto& expr) {
          Blue([&] { OutFmt() << "E" << ref << ":"; });
          Print(expr);
        },
        ctx_.DerefExpr(ref));
  }

 private:
  void Print(const FuncParam& param) {
    OutFmt() << "FuncParam {" << std::endl;
    Nested(
        [&] { OutFmt() << ".name = \"" << param.name << "\"" << std::endl; });
    OutFmt() << "}" << std::endl;
  }

  void Print(const VarDeclStmt& stmt) {
    Out() << "VarDeclStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".name = \"" << stmt.name << "\"" << std::endl;

      if (stmt.init.has_value()) {
        OutFmt() << ".init = {" << std::endl;
        Nested([&] { PrintExpr(*stmt.init); });
        OutFmt() << "}" << std::endl;
      }
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const VarAssignStmt& stmt) {
    Out() << "VarAssignStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".name = \"" << stmt.name << "\"" << std::endl;

      OutFmt() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const ArrayAssignStmt& stmt) {
    Out() << "ArrayAssignStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".name = \"" << stmt.name << "\"" << std::endl;

      OutFmt() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(stmt.index); });
      OutFmt() << "}" << std::endl;

      OutFmt() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const ReturnStmt& stmt) {
    Out() << "ReturnStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".value = {" << std::endl;
      Nested([&] { PrintExpr(stmt.value); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const DoStmt& stmt) {
    Out() << "DoStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const IfStmt& stmt) {
    Out() << "IfStmt {" << std::endl;
    Nested([&] {
      OutFmt() << ".cond = {" << std::endl;
      Nested([&] { PrintExpr(stmt.cond); });
      OutFmt() << "}" << std::endl;

      if (stmt.then_stmts.size() > 0) {
        OutFmt() << ".then_stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.then_stmts) {
            PrintExpr(stmt_ref);
          }
        });
        OutFmt() << "]" << std::endl;
      }
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const LoopStmt& stmt) {
    Out() << "LoopStmt {" << std::endl;
    Nested([&] {
      if (stmt.stmts.size() > 0) {
        OutFmt() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt : stmt.stmts) {
            PrintStmt(stmt);
          }
        });
        OutFmt() << "]" << std::endl;
      }
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const BreakStmt& stmt) { Out() << "BreakStmt {}" << std::endl; }

  void Print(const FuncCallExpr& expr) {
    Out() << "FuncCallExpr {" << std::endl;
    Nested([&] {
      OutFmt() << ".func_name = \"" << expr.func_name << "\"" << std::endl;

      if (expr.args.size() > 0) {
        OutFmt() << ".args = [" << std::endl;
        Nested([&] {
          for (ExprRef arg : expr.args) {
            PrintExpr(arg);
          }
        });
        OutFmt() << "]" << std::endl;
      }
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const IntLitExpr& expr) {
    Out() << "IntLitExpr {" << std::endl;
    Nested([&] { OutFmt() << ".value = " << expr.value << std::endl; });
    OutFmt() << "}" << std::endl;
  }

  void Print(const BoolLitExpr& expr) {
    Out() << "BoolLitExpr {" << std::endl;
    Nested([&] { OutFmt() << ".value = " << expr.value << std::endl; });
    OutFmt() << "}" << std::endl;
  }

  void Print(const StringLitExpr& expr) {
    Out() << "StringLitExpr {" << std::endl;
    Nested(
        [&] { OutFmt() << ".value = \"" << expr.value << "\"" << std::endl; });
    OutFmt() << "}" << std::endl;
  }

  void Print(const IdentExpr& expr) {
    Out() << "IdentExpr {" << std::endl;
    Nested([&] { OutFmt() << ".name = \"" << expr.name << "\"" << std::endl; });
    OutFmt() << "}" << std::endl;
  }

  void Print(const IndexExpr& expr) {
    Out() << "IndexExpr {" << std::endl;
    Nested([&] {
      OutFmt() << ".base = {" << std::endl;
      Nested([&] { PrintExpr(expr.base); });
      OutFmt() << "}" << std::endl;

      OutFmt() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(expr.index); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Print(const BinaryOpExpr& expr) {
    Out() << "BinaryOpExpr {" << std::endl;
    Nested([&] {
      switch (expr.op) {
        case BinaryOp::Add:
          OutFmt() << ".op = Add" << std::endl;
          break;
        case BinaryOp::Sub:
          OutFmt() << ".op = Sub" << std::endl;
          break;
        case BinaryOp::Mul:
          OutFmt() << ".op = Mul" << std::endl;
          break;
        case BinaryOp::Div:
          OutFmt() << ".op = Div" << std::endl;
          break;
        case BinaryOp::Mod:
          OutFmt() << ".op = Mod" << std::endl;
          break;
        case BinaryOp::Gt:
          OutFmt() << ".op = Gt" << std::endl;
          break;
        case BinaryOp::Lt:
          OutFmt() << ".op = Lt" << std::endl;
          break;
        case BinaryOp::Eq:
          OutFmt() << ".op = Eq" << std::endl;
          break;
        case BinaryOp::NotEq:
          OutFmt() << ".op = NotEq" << std::endl;
          break;
      }

      OutFmt() << ".lhs = {" << std::endl;
      Nested([&] { PrintExpr(expr.lhs); });
      OutFmt() << "}" << std::endl;

      OutFmt() << ".rhs = {" << std::endl;
      Nested([&] { PrintExpr(expr.rhs); });
      OutFmt() << "}" << std::endl;
    });
    OutFmt() << "}" << std::endl;
  }

  void Nested(std::function<void()> f) {
    indent_ += 2;
    std::invoke(f);
    indent_ -= 2;
  }

  void Blue(std::function<void()> f) {
    Out() << "\033[34m";
    std::invoke(f);
    Out() << "\033[0m ";
  }

  std::ostream& OutFmt() {
    for (int i = 0; i < indent_; ++i) std::cout << " ";
    return std::cout;
  }

  std::ostream& Out() { return std::cout; }

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
