#include "lucid/am/cfg_printer.h"

#include <ostream>
#include <string_view>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/cli/format.h"

namespace lucid {
namespace {

static constexpr int kIndentStep = 2;

void PrintPhi(int indent, const AbstractMachineControlFlowGraph::Phi& phi,
              std::ostream& out) {
  out << Indent(indent) << "φ(" << phi.dst << ") = (";
  for (bool has_printed_src = false; const auto& src : phi.srcs) {
    if (has_printed_src) out << ", ";
    out << src;
    has_printed_src = true;
  }
  out << ")" << "\n";
}

void PrintInst(int indent, const Instruction& inst, std::ostream& out) {
  std::visit(
      [&](const auto& inst) {
        using T = std::decay_t<decltype(inst)>;
        if constexpr (!std::is_same_v<T, Label>) {
          out << Indent(indent) << inst << "\n";
        }
      },
      inst);
}

void PrintBlock(int indent, const AbstractMachineControlFlowGraph::Block& block,
                std::ostream& out) {
  out << Indent(indent) << SetColor(Color::Blue) << "B" << block.ref.id()
      << ":\n"
      << ResetColor;

  for (const auto& phi : block.phis) {
    PrintPhi(indent + kIndentStep, phi, out);
  }
  for (const auto& inst : block.instructions) {
    PrintInst(indent + kIndentStep, inst, out);
  }
}

}  // namespace

void Print(std::string_view func_name,
           const AbstractMachineControlFlowGraph& amcfg, std::ostream& out) {
  out << SetColor(Color::Blue) << func_name << "(";
  for (bool has_printed_param = false; const auto& param : amcfg.params) {
    if (has_printed_param) out << ", ";
    out << param;
    has_printed_param = true;
  }
  out << "):\n" << ResetColor;

  for (const auto& block : amcfg.blocks()) {
    PrintBlock(kIndentStep, block, out);
  }
}

}  // namespace lucid
