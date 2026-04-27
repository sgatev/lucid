#include "lucid/syntax/ast_printer.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/syntax/ast.h"

namespace lucid {
namespace {

class AstPrinter {
 public:
  explicit AstPrinter(const SyntaxContext& ctx) : ctx_(ctx) {}

  void Print(const FuncDefStmt& stmt) {
    Out() << Indent() << "FuncDefStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(stmt.name) << "\""
            << std::endl;

      if (stmt.params.size() > 0) {
        Out() << Indent() << ".params = [" << std::endl;
        Nested([&] {
          for (ParamRef param_ref : stmt.params) {
            Print(ctx_.DerefParam(param_ref));
          }
        });
        Out() << Indent() << "]" << std::endl;
      }

      if (stmt.stmts.size() > 0) {
        Out() << Indent() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.stmts) {
            PrintStmt(stmt_ref);
          }
        });
        Out() << Indent() << "]" << std::endl;
      }
    });
    Out() << Indent() << "}" << std::endl;
  }

  void PrintStmt(StmtRef ref) {
    std::visit(
        [&](const auto& stmt) {
          Blue([&] { Out() << Indent() << "S" << ref << ": "; });
          Print(stmt);
        },
        ctx_.DerefStmt(ref));
  }

  void PrintExpr(ExprRef ref) {
    std::visit(
        [&](const auto& expr) {
          Blue([&] { Out() << Indent() << "E" << ref << ": "; });
          Print(expr);
        },
        ctx_.DerefExpr(ref));
  }

 private:
  void Print(const FuncParam& param) {
    Out() << Indent() << "FuncParam {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(param.name) << "\""
            << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const VarDeclStmt& stmt) {
    Out() << "VarDeclStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(stmt.name) << "\""
            << std::endl;

      if (stmt.init.has_value()) {
        Out() << Indent() << ".init = {" << std::endl;
        Nested([&] { PrintExpr(*stmt.init); });
        Out() << Indent() << "}" << std::endl;
      }
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const VarAssignStmt& stmt) {
    Out() << "VarAssignStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(stmt.name) << "\""
            << std::endl;

      Out() << Indent() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const ArrayAssignStmt& stmt) {
    Out() << "ArrayAssignStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(stmt.name) << "\""
            << std::endl;

      Out() << Indent() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(stmt.index); });
      Out() << Indent() << "}" << std::endl;

      Out() << Indent() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const ReturnStmt& stmt) {
    Out() << "ReturnStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".value = {" << std::endl;
      Nested([&] { PrintExpr(stmt.value); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const DoStmt& stmt) {
    Out() << "DoStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".expr = {" << std::endl;
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const IfStmt& stmt) {
    Out() << "IfStmt {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".cond = {" << std::endl;
      Nested([&] { PrintExpr(stmt.cond); });
      Out() << Indent() << "}" << std::endl;

      if (stmt.then_stmts.size() > 0) {
        Out() << Indent() << ".then_stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt_ref : stmt.then_stmts) {
            PrintStmt(stmt_ref);
          }
        });
        Out() << Indent() << "]" << std::endl;
      }
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const LoopStmt& stmt) {
    Out() << "LoopStmt {" << std::endl;
    Nested([&] {
      if (stmt.stmts.size() > 0) {
        Out() << Indent() << ".stmts = [" << std::endl;
        Nested([&] {
          for (StmtRef stmt : stmt.stmts) {
            PrintStmt(stmt);
          }
        });
        Out() << Indent() << "]" << std::endl;
      }
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const BreakStmt& stmt) { Out() << "BreakStmt {}" << std::endl; }

  void Print(const FuncCallExpr& expr) {
    Out() << "FuncCallExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".func_name = \"" << ctx_.DerefIdent(expr.func_name)
            << "\"" << std::endl;

      if (expr.args.size() > 0) {
        Out() << Indent() << ".args = [" << std::endl;
        Nested([&] {
          for (ExprRef arg : expr.args) {
            PrintExpr(arg);
          }
        });
        Out() << Indent() << "]" << std::endl;
      }
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const IntLitExpr& expr) {
    Out() << "IntLitExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".value = " << ctx_.DerefIdent(expr.value)
            << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const BoolLitExpr& expr) {
    Out() << "BoolLitExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".value = " << ctx_.DerefIdent(expr.value)
            << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const StringLitExpr& expr) {
    Out() << "StringLitExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".value = ";
      Green([&] { Out() << ctx_.DerefIdent(expr.value); });
      Out() << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const IdentExpr& expr) {
    Out() << "IdentExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".name = \"" << ctx_.DerefIdent(expr.name) << "\""
            << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const IndexExpr& expr) {
    Out() << "IndexExpr {" << std::endl;
    Nested([&] {
      Out() << Indent() << ".base = {" << std::endl;
      Nested([&] { PrintExpr(expr.base); });
      Out() << Indent() << "}" << std::endl;

      Out() << Indent() << ".index = {" << std::endl;
      Nested([&] { PrintExpr(expr.index); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Print(const BinaryOpExpr& expr) {
    Out() << "BinaryOpExpr {" << std::endl;
    Nested([&] {
      switch (expr.op) {
        case BinaryOp::Add:
          Out() << Indent() << ".op = Add" << std::endl;
          break;
        case BinaryOp::Sub:
          Out() << Indent() << ".op = Sub" << std::endl;
          break;
        case BinaryOp::Mul:
          Out() << Indent() << ".op = Mul" << std::endl;
          break;
        case BinaryOp::Div:
          Out() << Indent() << ".op = Div" << std::endl;
          break;
        case BinaryOp::Mod:
          Out() << Indent() << ".op = Mod" << std::endl;
          break;
        case BinaryOp::Gt:
          Out() << Indent() << ".op = Gt" << std::endl;
          break;
        case BinaryOp::Lt:
          Out() << Indent() << ".op = Lt" << std::endl;
          break;
        case BinaryOp::Eq:
          Out() << Indent() << ".op = Eq" << std::endl;
          break;
        case BinaryOp::NotEq:
          Out() << Indent() << ".op = NotEq" << std::endl;
          break;
      }

      Out() << Indent() << ".lhs = {" << std::endl;
      Nested([&] { PrintExpr(expr.lhs); });
      Out() << Indent() << "}" << std::endl;

      Out() << Indent() << ".rhs = {" << std::endl;
      Nested([&] { PrintExpr(expr.rhs); });
      Out() << Indent() << "}" << std::endl;
    });
    Out() << Indent() << "}" << std::endl;
  }

  void Nested(std::function<void()> f) {
    indent_.push_back(' ');
    indent_.push_back(' ');
    std::invoke(f);
    indent_.pop_back();
    indent_.pop_back();
  }

  void Blue(std::function<void()> f) {
    Out() << "\033[34m";
    std::invoke(f);
    Out() << "\033[0m";
  }

  void LightBlue(std::function<void()> f) {
    Out() << "\033[36m";
    std::invoke(f);
    Out() << "\033[0m";
  }

  void Green(std::function<void()> f) {
    Out() << "\033[32m";
    std::invoke(f);
    Out() << "\033[0m";
  }

  std::string_view Indent() {
    return std::string_view(indent_.data(), indent_.size());
  }

  std::ostream& Out() { return std::cout; }

  const SyntaxContext ctx_;
  std::vector<char> indent_;
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
