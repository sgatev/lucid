#include "lucid/cfg_printer.h"

#include <functional>
#include <iostream>
#include <variant>

#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

void Blue(std::function<void()> f) {
  std::cout << "\033[34m";
  std::invoke(f);
  std::cout << "\033[0m";
}

void Print(const SyntaxContext& ctx, const ControlFlowGraph& graph) {
  Blue([&] { std::cout << graph.func_name << ":" << std::endl; });
  for (const auto& block : graph.blocks()) {
    Blue([&] { std::cout << "  B" << block.id << ": "; });
    std::cout << "{" << std::endl;

    if (!block.sequences.empty()) {
      std::cout << "    .sequences = [" << std::endl;
      for (const auto& seq : block.sequences) {
        for (const auto& expr_ref : seq.expressions) {
          Blue([&] { std::cout << "      E" << expr_ref << ": "; });

          const auto& expr = ctx.DerefExpr(expr_ref);
          if (std::holds_alternative<FuncCallExpr>(expr)) {
            std::cout << "FuncCallExpr";
          } else if (std::holds_alternative<IntLitExpr>(expr)) {
            std::cout << "IntLitExpr";
          } else if (std::holds_alternative<BoolLitExpr>(expr)) {
            std::cout << "BoolLitExpr";
          } else if (std::holds_alternative<StringLitExpr>(expr)) {
            std::cout << "StringLitExpr";
          } else if (std::holds_alternative<IdentExpr>(expr)) {
            std::cout << "IdentExpr";
          } else if (std::holds_alternative<IndexExpr>(expr)) {
            std::cout << "IndexExpr";
          } else if (std::holds_alternative<BinaryOpExpr>(expr)) {
            std::cout << "BinaryOpExpr";
          }

          std::cout << std::endl;
        }
        if (seq.stmt.has_value()) {
          Blue([&] { std::cout << "      S" << *seq.stmt << ": "; });

          const auto& stmt = ctx.DerefStmt(*seq.stmt);
          if (std::holds_alternative<VarDeclStmt>(stmt)) {
            std::cout << "VarDeclStmt";
          } else if (std::holds_alternative<VarAssignStmt>(stmt)) {
            std::cout << "VarAssignStmt";
          } else if (std::holds_alternative<ArrayAssignStmt>(stmt)) {
            std::cout << "ArrayAssignStmt";
          } else if (std::holds_alternative<FuncDefStmt>(stmt)) {
            std::cout << "FuncDefStmt";
          } else if (std::holds_alternative<ReturnStmt>(stmt)) {
            std::cout << "ReturnStmt";
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

    std::cout << "  }" << std::endl;
  }
  std::cout << std::endl;
}

}  // namespace lucid
