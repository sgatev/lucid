#include "lucid/am_cfg.h"

#include <span>

#include "lucid/am.h"
#include "lucid/core/container/hash_map.h"

namespace lucid {

AbstractMachineControlFlowGraph BuildAbstractMachineControlFlowGraph(
    const std::vector<Instruction>& instructions) {
  static constexpr std::uint32_t kSourceBlockId = 0;
  static constexpr std::uint32_t kSinkBlockId = 1;

  HashMap<std::uint32_t, std::uint32_t> block_ids;
  block_ids.Insert(kSourceBlockId, kSourceBlockId);
  block_ids.Insert(kSinkBlockId, kSinkBlockId);

  HashMap<std::uint32_t, AbstractMachineControlFlowGraph::Block> blocks;
  blocks.Insert(kSourceBlockId,
                AbstractMachineControlFlowGraph::Block(kSourceBlockId));
  blocks.Insert(kSinkBlockId,
                AbstractMachineControlFlowGraph::Block(kSinkBlockId));
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (auto* label = std::get_if<Label>(&instructions[i])) {
      block_ids.Insert(label->id + 2, block_ids.size());
      std::size_t block_id = *block_ids.Find(label->id + 2);
      blocks.Insert(block_id, AbstractMachineControlFlowGraph::Block(block_id));
    }
  }

  std::uint32_t current_block_id = kSourceBlockId;
  std::uint32_t current_inst_start = 0;
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (auto* label = std::get_if<Label>(&instructions[i])) {
      blocks.Find(current_block_id)
          ->next_blocks_.Insert(AbstractMachineControlFlowGraph::BlockRef(
              *block_ids.Find(label->id + 2)));
      blocks.Find(*block_ids.Find(label->id + 2))
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));

      blocks.Find(current_block_id)->instructions_ =
          std::span<const Instruction>(
              instructions.begin() + current_inst_start,
              i - current_inst_start);

      current_block_id = *block_ids.Find(label->id + 2);
      current_inst_start = i + 1;
    } else if (auto* jump = std::get_if<CondJump>(&instructions[i])) {
      std::uint32_t then_block_id = *block_ids.Find(jump->then_label + 2);
      std::uint32_t else_block_id = *block_ids.Find(jump->else_label + 2);

      blocks.Find(current_block_id)
          ->next_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(then_block_id));
      blocks.Find(then_block_id)
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));

      blocks.Find(current_block_id)
          ->next_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(else_block_id));
      blocks.Find(else_block_id)
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));
    } else if (auto* jump = std::get_if<UncondJump>(&instructions[i])) {
      std::uint32_t next_block_id = *block_ids.Find(jump->label + 2);

      blocks.Find(current_block_id)
          ->next_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(next_block_id));
      blocks.Find(next_block_id)
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));
    } else if (std::holds_alternative<Return>(instructions[i])) {
      blocks.Find(current_block_id)
          ->next_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(kSinkBlockId));
      blocks.Find(kSinkBlockId)
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));
    }
  }

  blocks.Find(current_block_id)->instructions_ =
      std::span<const Instruction>(instructions.begin() + current_inst_start,
                                   instructions.size() - current_inst_start);
  blocks.Find(current_block_id)
      ->next_blocks_.Insert(
          AbstractMachineControlFlowGraph::BlockRef(kSinkBlockId));
  blocks.Find(kSinkBlockId)
      ->previous_blocks_.Insert(
          AbstractMachineControlFlowGraph::BlockRef(current_block_id));

  return AbstractMachineControlFlowGraph(std::move(blocks));
}

}  // namespace lucid
