#include "lucid/am/cfg_printer.h"

#include <cstddef>
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
  std::visit([&](const auto& inst) { out << Indent(indent) << inst << "\n"; },
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
  const std::size_t in_registers = amcfg.RegisterParams().size();
  for (std::size_t i = 0; i < amcfg.params.size(); ++i) {
    if (i > 0) out << ", ";
    out << amcfg.params[i];

    // A parameter the caller leaves on the stack says which slot it stands
    // in, because nothing holds it and every read of it loads from there.
    if (i >= in_registers) out << "@" << i - in_registers;
  }
  out << "):\n" << ResetColor;

  for (const auto& block : amcfg.Blocks()) {
    PrintBlock(kIndentStep, block, out);
  }
}

}  // namespace lucid
