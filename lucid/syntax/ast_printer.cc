#include "lucid/syntax/ast_printer.h"

#include <functional>
#include <ostream>
#include <variant>

#include "lucid/core/cli/format.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

class AstPrinter {
 public:
  AstPrinter(const SyntaxContext& syn_ctx, std::ostream& out)
      : syn_ctx_(syn_ctx), out_(out) {}

  void Print(const FuncDefStmt& stmt) {
    Out() << Indent(indent_) << "FuncDefStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(stmt.name)
            << "\"\n";

      if (stmt.params.size() > 0) {
        Out() << Indent(indent_) << ".params = [\n";
        Nested([&] {
          for (ParamRef param_ref : stmt.params) {
            Print(syn_ctx_.DerefParam(param_ref));
          }
        });
        Out() << Indent(indent_) << "]\n";
      }

      if (stmt.stmts.size() > 0) {
        Out() << Indent(indent_) << ".stmts = [\n";
        Nested([&] {
          for (StmtRef stmt_ref : stmt.stmts) {
            PrintStmt(stmt_ref);
          }
        });
        Out() << Indent(indent_) << "]\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const TypeDefStmt& stmt) {
    Out() << Indent(indent_) << "TypeDefStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(stmt.name)
            << "\"\n";

      const Type& type = syn_ctx_.DerefType(stmt.type);
      if (const auto* tuple = std::get_if<TupleType>(&type)) {
        Out() << Indent(indent_) << ".type = tuple {\n";
        Nested([&] {
          if (tuple->fields.size() > 0) {
            Out() << Indent(indent_) << ".fields = [\n";
            Nested([&] {
              for (ParamRef param_ref : tuple->fields) {
                Print(syn_ctx_.DerefParam(param_ref));
              }
            });
            Out() << Indent(indent_) << "]\n";
          }
        });
        Out() << Indent(indent_) << "}\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void PrintStmt(StmtRef ref) {
    std::visit(
        [&](const auto& stmt) {
          Out() << SetColor(Color::Blue) << Indent(indent_) << "S" << ref
                << ": " << ResetColor;
          Print(stmt);
        },
        syn_ctx_.DerefStmt(ref));
  }

  void PrintExpr(ExprRef ref) {
    std::visit(
        [&](const auto& expr) {
          Out() << SetColor(Color::Blue) << Indent(indent_) << "E" << ref
                << ": " << ResetColor;
          Print(expr);
        },
        syn_ctx_.DerefExpr(ref));
  }

 private:
  void Print(const FuncParam& param) {
    Out() << Indent(indent_) << "FuncParam {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \""
            << syn_ctx_.DerefIdent(param.name) << "\"\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const VarDeclStmt& stmt) {
    Out() << "VarDeclStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(stmt.name)
            << "\"\n";

      if (stmt.init.has_value()) {
        Out() << Indent(indent_) << ".init = {\n";
        Nested([&] { PrintExpr(*stmt.init); });
        Out() << Indent(indent_) << "}\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const VarAssignStmt& stmt) {
    Out() << "VarAssignStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(stmt.name)
            << "\"\n";

      Out() << Indent(indent_) << ".expr = {\n";
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const ArrayAssignStmt& stmt) {
    Out() << "ArrayAssignStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(stmt.name)
            << "\"\n";

      Out() << Indent(indent_) << ".index = {\n";
      Nested([&] { PrintExpr(stmt.index); });
      Out() << Indent(indent_) << "}\n";

      Out() << Indent(indent_) << ".expr = {\n";
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const FieldAssignStmt& stmt) {
    Out() << "TupleFieldAssignStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".tuple_name = E" << stmt.base << "\n";

      Out() << Indent(indent_) << ".field_name = \""
            << syn_ctx_.DerefIdent(stmt.field_name) << "\"\n";

      Out() << Indent(indent_) << ".expr = {\n";
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const ReturnStmt& stmt) {
    Out() << "ReturnStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".value = {\n";
      Nested([&] { PrintExpr(stmt.value); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const DoStmt& stmt) {
    Out() << "DoStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".expr = {\n";
      Nested([&] { PrintExpr(stmt.expr); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const IfStmt& stmt) {
    Out() << "IfStmt {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".cond = {\n";
      Nested([&] { PrintExpr(stmt.cond); });
      Out() << Indent(indent_) << "}\n";

      if (stmt.then_stmts.size() > 0) {
        Out() << Indent(indent_) << ".then_stmts = [\n";
        Nested([&] {
          for (StmtRef stmt_ref : stmt.then_stmts) {
            PrintStmt(stmt_ref);
          }
        });
        Out() << Indent(indent_) << "]\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const LoopStmt& stmt) {
    Out() << "LoopStmt {\n";
    Nested([&] {
      if (stmt.stmts.size() > 0) {
        Out() << Indent(indent_) << ".stmts = [\n";
        Nested([&] {
          for (StmtRef stmt : stmt.stmts) {
            PrintStmt(stmt);
          }
        });
        Out() << Indent(indent_) << "]\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const BreakStmt& stmt) { Out() << "BreakStmt {}\n"; }

  void Print(const FuncCallExpr& expr) {
    Out() << "FuncCallExpr {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".func_name = \""
            << syn_ctx_.DerefIdent(expr.func_name) << "\"\n";

      if (expr.args.size() > 0) {
        Out() << Indent(indent_) << ".args = [\n";
        Nested([&] {
          for (ExprRef arg : expr.args) {
            PrintExpr(arg);
          }
        });
        Out() << Indent(indent_) << "]\n";
      }
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const IntLitExpr& expr) {
    Out() << "IntLitExpr {\n";
    Nested(
        [&] { Out() << Indent(indent_) << ".value = " << expr.value << "\n"; });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const BoolLitExpr& expr) {
    Out() << "BoolLitExpr {\n";
    Nested(
        [&] { Out() << Indent(indent_) << ".value = " << expr.value << "\n"; });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const StringLitExpr& expr) {
    Out() << "StringLitExpr {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".value = " << SetColor(Color::Green)
            << syn_ctx_.DerefIdent(expr.value) << ResetColor << "\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const IdentExpr& expr) {
    Out() << "IdentExpr {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".name = \"" << syn_ctx_.DerefIdent(expr.name)
            << "\"\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const IndexExpr& expr) {
    Out() << "IndexExpr {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".base = {\n";
      Nested([&] { PrintExpr(expr.base); });
      Out() << Indent(indent_) << "}\n";

      Out() << Indent(indent_) << ".index = {\n";
      Nested([&] { PrintExpr(expr.index); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const FieldAccessExpr& expr) {
    Out() << "FieldAccessExpr {\n";
    Nested([&] {
      Out() << Indent(indent_) << ".base = {\n";
      Nested([&] { PrintExpr(expr.base); });
      Out() << Indent(indent_) << "}\n";

      Out() << Indent(indent_) << ".field_name = '"
            << syn_ctx_.DerefIdent(expr.field_name) << "'\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Print(const BinaryOpExpr& expr) {
    Out() << "BinaryOpExpr {\n";
    Nested([&] {
      switch (expr.op) {
        case BinaryOp::Add:
          Out() << Indent(indent_) << ".op = Add\n";
          break;
        case BinaryOp::Sub:
          Out() << Indent(indent_) << ".op = Sub\n";
          break;
        case BinaryOp::Mul:
          Out() << Indent(indent_) << ".op = Mul\n";
          break;
        case BinaryOp::Div:
          Out() << Indent(indent_) << ".op = Div\n";
          break;
        case BinaryOp::Mod:
          Out() << Indent(indent_) << ".op = Mod\n";
          break;
        case BinaryOp::Gt:
          Out() << Indent(indent_) << ".op = Gt\n";
          break;
        case BinaryOp::Lt:
          Out() << Indent(indent_) << ".op = Lt\n";
          break;
        case BinaryOp::Eq:
          Out() << Indent(indent_) << ".op = Eq\n";
          break;
        case BinaryOp::NotEq:
          Out() << Indent(indent_) << ".op = NotEq\n";
          break;
      }

      Out() << Indent(indent_) << ".lhs = {\n";
      Nested([&] { PrintExpr(expr.lhs); });
      Out() << Indent(indent_) << "}\n";

      Out() << Indent(indent_) << ".rhs = {\n";
      Nested([&] { PrintExpr(expr.rhs); });
      Out() << Indent(indent_) << "}\n";
    });
    Out() << Indent(indent_) << "}\n";
  }

  void Nested(std::function<void()> f) {
    indent_ += 2;
    std::invoke(f);
    indent_ -= 2;
  }

  std::ostream& Out() { return out_; }

  const SyntaxContext syn_ctx_;
  std::ostream& out_;
  int indent_ = 0;
};

}  // namespace

void Print(const SyntaxContext ctx, const FuncDefStmt& stmt,
           std::ostream& out) {
  AstPrinter(ctx, out).Print(stmt);
}

void PrintStmt(const SyntaxContext ctx, StmtRef ref, std::ostream& out) {
  AstPrinter(ctx, out).PrintStmt(ref);
}

void PrintExpr(const SyntaxContext ctx, ExprRef ref, std::ostream& out) {
  AstPrinter(ctx, out).PrintExpr(ref);
}

}  // namespace lucid
