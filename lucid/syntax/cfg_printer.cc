#include "lucid/syntax/cfg_printer.h"

#include <functional>
#include <iostream>
#include <variant>

#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

void Blue(std::function<void()> f) {
  std::cout << "\033[34m";
  std::invoke(f);
  std::cout << "\033[0m";
}

}  // namespace

void Print(const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg) {
  Blue(
      [&] { std::cout << ctx.DerefIdent(scfg.func_name) << ":" << std::endl; });
  for (const auto& block : scfg.blocks()) {
    Blue([&] { std::cout << "  B" << block.ref << ": "; });
    std::cout << "{" << std::endl;

    if (!block.phis.empty()) {
      std::cout << "    .phis = [" << std::endl;
      for (const auto& phi : block.phis) {
        std::cout << "      " << ctx.DerefIdent(phi.name) << " = φ(";
        for (int i = 0; i < phi.args.size(); ++i) {
          if (i > 0) std::cout << ", ";
          std::cout << ctx.DerefIdent(phi.args[i]);
        }
        std::cout << ")" << std::endl;
      }
      std::cout << "    ]" << std::endl;
    }

    if (!block.sequences.empty()) {
      std::cout << "    .sequences = [" << std::endl;
      for (const auto& seq : block.sequences) {
        for (const auto& expr_ref : seq.expressions) {
          Blue([&] { std::cout << "      E" << expr_ref << ": "; });

          const auto& expr = ctx.DerefExpr(expr_ref);
          if (std::holds_alternative<FuncCallExpr>(expr)) {
            std::cout << "FuncCallExpr";
          } else if (auto* s = std::get_if<IntLitExpr>(&expr)) {
            std::cout << "IntLitExpr { .value = " << ctx.DerefIdent(s->value)
                      << " }";
          } else if (std::holds_alternative<BoolLitExpr>(expr)) {
            std::cout << "BoolLitExpr";
          } else if (std::holds_alternative<StringLitExpr>(expr)) {
            std::cout << "StringLitExpr";
          } else if (auto* e = std::get_if<IdentExpr>(&expr)) {
            std::cout << "IdentExpr { .name = '" << ctx.DerefIdent(e->name)
                      << "' }";
          } else if (std::holds_alternative<IndexExpr>(expr)) {
            std::cout << "IndexExpr";
          } else if (auto* s = std::get_if<BinaryOpExpr>(&expr)) {
            std::cout << "BinaryOpExpr { .op = " << to_string(s->op)
                      << ", .lhs = E" << s->lhs << ", .rhs = E" << s->rhs
                      << " }";
          }

          std::cout << std::endl;
        }
        if (seq.stmt.has_value()) {
          Blue([&] { std::cout << "      S" << *seq.stmt << ": "; });

          const auto& stmt = ctx.DerefStmt(*seq.stmt);
          if (auto* s = std::get_if<VarDeclStmt>(&stmt)) {
            std::cout << "VarDeclStmt { .name = '" << ctx.DerefIdent(s->name)
                      << "'";
            if (s->init.has_value()) std::cout << ", .init = E" << *s->init;
            std::cout << " }";
          } else if (auto* s = std::get_if<VarAssignStmt>(&stmt)) {
            std::cout << "VarAssignStmt { .name = '" << ctx.DerefIdent(s->name)
                      << "', .expr = E" << s->expr << " }";
          } else if (std::holds_alternative<ArrayAssignStmt>(stmt)) {
            std::cout << "ArrayAssignStmt";
          } else if (std::holds_alternative<FuncDefStmt>(stmt)) {
            std::cout << "FuncDefStmt";
          } else if (auto* s = std::get_if<ReturnStmt>(&stmt)) {
            std::cout << "ReturnStmt { .value = E" << s->value << " }";
          } else if (std::holds_alternative<DoStmt>(stmt)) {
            std::cout << "DoStmt";
          } else if (std::holds_alternative<BreakStmt>(stmt)) {
            std::cout << "BreakStmt";
          }

          std::cout << std::endl;
        }
      }
      std::cout << "    ]" << std::endl;
    }

    if (!block.next.empty()) {
      std::cout << "    .next = [" << std::endl;
      for (const auto& next : block.next) {
        Blue([&] { std::cout << "      B" << next << std::endl; });
      }
      std::cout << "    ]" << std::endl;
    }

    if (!block.preds.empty()) {
      std::cout << "    .preds = [" << std::endl;
      for (const auto& pred : block.preds) {
        Blue([&] { std::cout << "      B" << pred << std::endl; });
      }
      std::cout << "    ]" << std::endl;
    }

    std::cout << "  }" << std::endl;
  }
  std::cout << std::endl;
}

}  // namespace lucid
