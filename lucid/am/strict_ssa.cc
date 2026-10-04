#include "lucid/am/strict_ssa.h"

#include <cstddef>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/graph/dominator.h"

namespace lucid {
namespace {

using BlockRef = AbstractMachineControlFlowGraph::BlockRef;

// Where a register is written: its block, and how far into the block. A
// parameter and a phi function's result are written where the block starts,
// at 0, and the instruction at `i` writes at `i + 1`.
struct Write {
  BlockRef block;
  std::size_t at;
};

}  // namespace

std::expected<void, std::string> CheckStrictSsa(
    const AbstractMachineControlFlowGraph& am_cfg) {
  const auto idoms = ComputeImmediateDominators(am_cfg);
  // A block nothing reaches has no dominators, so whether a write comes
  // first on every way there has no answer.
  for (const auto& block : am_cfg.Blocks()) {
    if (!idoms[block.ref.id()].has_value()) {
      return std::unexpected(std::format(
          "block {} is not reached from the entry", block.ref.id()));
    }
  }
  // Whether every way from the entry to `to` passes through `from`.
  const auto dominates = [&](BlockRef from, BlockRef to) {
    while (true) {
      if (to == from) return true;
      const BlockRef up = *idoms[to.id()];
      if (up == to) return false;
      to = up;
    }
  };

  std::vector<std::optional<Write>> writes(am_cfg.next_free_reg_id);
  const auto write = [&](Reg reg,
                         Write where) -> std::expected<void, std::string> {
    if (reg.id < 0 || std::size_t(reg.id) >= writes.size()) {
      return std::unexpected(std::format(
          "register {} is not one of the function's registers", reg.id));
    }
    if (writes[reg.id].has_value()) {
      return std::unexpected(
          std::format("register {} is written more than once", reg.id));
    }
    writes[reg.id] = where;
    return {};
  };
  // `at` is how far into `block` the read stands, counted the way a write's
  // place is.
  const auto read = [&](Reg reg, BlockRef block,
                        std::size_t at) -> std::expected<void, std::string> {
    const std::optional<Write> written =
        reg.id >= 0 && std::size_t(reg.id) < writes.size() ? writes[reg.id]
                                                           : std::nullopt;
    if (!written.has_value()) {
      return std::unexpected(
          std::format("register {} is read in block {} but never written",
                      reg.id, block.id()));
    }
    const bool first = written->block == block
                           ? written->at < at
                           : dominates(written->block, block);
    if (!first) {
      return std::unexpected(std::format(
          "register {} is read in block {} where its write in block {} does "
          "not come first on every way there",
          reg.id, block.id(), written->block.id()));
    }
    return {};
  };

  std::expected<void, std::string> result;
  for (Reg param : am_cfg.params) {
    result = result.and_then([&] { return write(param, {am_cfg.first, 0}); });
  }
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& phi : block.phis) {
      result = result.and_then([&] { return write(phi.dst, {block.ref, 0}); });
    }
    std::size_t at = 1;
    for (const auto& inst : block.instructions) {
      if (const auto target = GetTargetRegister(inst); target.has_value()) {
        result =
            result.and_then([&] { return write(*target, {block.ref, at}); });
      }
      ++at;
    }
  }
  if (!result.has_value()) return result;

  for (const auto& block : am_cfg.Blocks()) {
    std::size_t at = 1;
    for (const auto& inst : block.instructions) {
      ForEachSourceRegister(inst, [&](Reg reg) {
        result = result.and_then([&] { return read(reg, block.ref, at); });
      });
      ++at;
    }
    if (block.branch_cond.has_value()) {
      result = result.and_then(
          [&] { return read(*block.branch_cond, block.ref, at); });
    }
    // An argument is read where control leaves the block it comes from,
    // after everything that block does.
    for (const auto& phi : block.phis) {
      for (std::size_t i = 0; i < phi.srcs.size(); ++i) {
        const BlockRef pred = block.preds[i];
        const std::size_t end = am_cfg.GetBlock(pred).instructions.size() + 1;
        result = result.and_then([&] { return read(phi.srcs[i], pred, end); });
      }
    }
  }
  return result;
}

}  // namespace lucid
