#include "lucid/syntax/cfg_printer.h"

#include <ostream>
#include <span>
#include <variant>

#include "lucid/core/cli/format.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

static constexpr int kIndentStep = 2;

void PrintPhi(int indent, const SyntaxContext& ctx,
              const SyntaxControlFlowGraph::Phi& phi, std::ostream& out) {
  out << Indent(indent) << ctx.DerefIdent(phi.name) << " = φ(";
  for (bool has_printed_arg = false; const auto& arg : phi.args) {
    if (has_printed_arg) out << ", ";
    out << ctx.DerefIdent(arg);
    has_printed_arg = true;
  }
  out << ")\n";
}

void PrintPhis(int indent, const SyntaxContext& ctx,
               const SyntaxControlFlowGraph& scfg,
               std::span<const SyntaxControlFlowGraph::PhiRef> phis,
               std::ostream& out) {
  if (phis.empty()) return;

  out << Indent(indent) << ".phis = [\n";
  for (auto phi_ref : phis) PrintPhi(indent + 2, ctx, scfg.deref(phi_ref), out);
  out << Indent(indent) << "]\n";
}

void PrintSequence(int indent, const SyntaxContext& ctx,
                   const SyntaxControlFlowGraph::Sequence& seq,
                   std::ostream& out) {
  for (const auto& expr_ref : seq.expressions) {
    out << Indent(indent) << SetColor(Color::Blue) << "E" << expr_ref << ": "
        << ResetColor;

    const auto& expr = ctx.DerefExpr(expr_ref);
    if (std::holds_alternative<FuncCallExpr>(expr)) {
      out << "FuncCallExpr";
    } else if (auto* s = std::get_if<IntLitExpr>(&expr)) {
      out << "IntLitExpr { .value = " << ctx.DerefIdent(s->value) << " }";
    } else if (std::holds_alternative<BoolLitExpr>(expr)) {
      out << "BoolLitExpr";
    } else if (std::holds_alternative<StringLitExpr>(expr)) {
      out << "StringLitExpr";
    } else if (auto* e = std::get_if<IdentExpr>(&expr)) {
      out << "IdentExpr { .name = '" << ctx.DerefIdent(e->name) << "' }";
    } else if (std::holds_alternative<IndexExpr>(expr)) {
      out << "IndexExpr";
    } else if (auto* s = std::get_if<BinaryOpExpr>(&expr)) {
      out << "BinaryOpExpr { .op = " << to_string(s->op) << ", .lhs = E"
          << s->lhs << ", .rhs = E" << s->rhs << " }";
    }

    out << "\n";
  }
  if (seq.stmt.has_value()) {
    out << Indent(indent) << SetColor(Color::Blue) << "S" << *seq.stmt << ": "
        << ResetColor;

    const auto& stmt = ctx.DerefStmt(*seq.stmt);
    if (auto* s = std::get_if<VarDeclStmt>(&stmt)) {
      out << "VarDeclStmt { .name = '" << ctx.DerefIdent(s->name) << "'";
      if (s->init.has_value()) out << ", .init = E" << *s->init;
      out << " }";
    } else if (auto* s = std::get_if<VarAssignStmt>(&stmt)) {
      out << "VarAssignStmt { .name = '" << ctx.DerefIdent(s->name)
          << "', .expr = E" << s->expr << " }";
    } else if (std::holds_alternative<ArrayAssignStmt>(stmt)) {
      out << "ArrayAssignStmt";
    } else if (std::holds_alternative<FuncDefStmt>(stmt)) {
      out << "FuncDefStmt";
    } else if (auto* s = std::get_if<ReturnStmt>(&stmt)) {
      out << "ReturnStmt { .value = E" << s->value << " }";
    } else if (std::holds_alternative<DoStmt>(stmt)) {
      out << "DoStmt";
    } else if (std::holds_alternative<BreakStmt>(stmt)) {
      out << "BreakStmt";
    }

    out << "\n";
  }
}

void PrintSequences(int indent, const SyntaxContext& ctx,
                    std::span<const SyntaxControlFlowGraph::Sequence> seqs,
                    std::ostream& out) {
  if (seqs.empty()) return;

  out << Indent(indent) << ".sequences = [\n";
  for (const auto& seq : seqs) {
    PrintSequence(indent + kIndentStep, ctx, seq, out);
  }
  out << Indent(indent) << "]\n";
}

void PrintBlockRef(int indent, SyntaxControlFlowGraph::BlockRef block_ref,
                   std::ostream& out) {
  out << Indent(indent) << SetColor(Color::Blue) << "B" << block_ref
      << ResetColor << "\n";
}

void PrintNext(int indent,
               std::span<const SyntaxControlFlowGraph::BlockRef> next,
               std::ostream& out) {
  if (next.empty()) return;

  out << Indent(indent) << ".next = [\n";
  for (const auto& next : next) PrintBlockRef(indent, next, out);
  out << Indent(indent) << "]\n";
}

void PrintPreds(int indent,
                std::span<const SyntaxControlFlowGraph::BlockRef> preds,
                std::ostream& out) {
  if (preds.empty()) return;

  out << Indent(indent) << ".preds = [\n";
  for (const auto& pred : preds) PrintBlockRef(indent, pred, out);
  out << Indent(indent) << "]\n";
}

void PrintBlock(int indent, const SyntaxContext& ctx,
                const SyntaxControlFlowGraph& scfg,
                const SyntaxControlFlowGraph::Block& block, std::ostream& out) {
  out << Indent(indent) << SetColor(Color::Blue) << "B" << block.ref << ":"
      << ResetColor << " {\n";
  PrintPhis(indent + kIndentStep, ctx, scfg, block.phis, out);
  PrintSequences(indent + kIndentStep, ctx, block.sequences, out);
  PrintNext(indent + kIndentStep, block.next, out);
  PrintPreds(indent + kIndentStep, block.preds, out);
  out << Indent(indent) << "}\n";
}

}  // namespace

void Print(const SyntaxContext& ctx, const SyntaxControlFlowGraph& scfg,
           std::ostream& out) {
  out << SetColor(Color::Blue) << ctx.DerefIdent(scfg.func_name) << "(";
  for (bool has_printed_param = false; auto param_ref : scfg.func_params) {
    if (has_printed_param) out << ", ";
    out << ctx.DerefIdent(ctx.DerefParam(param_ref).name);
    has_printed_param = true;
  }
  out << ")" << ResetColor << " {\n";
  for (const auto& block : scfg.blocks()) {
    PrintBlock(kIndentStep, ctx, scfg, block, out);
  }
  out << "}\n";
}

}  // namespace lucid
