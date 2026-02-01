#include "lucid/am_cfg.h"

namespace lucid {

AbstractMachineControlFlowGraph BuildAbstractMachineControlFlowGraph(
    const std::vector<Instruction>& instructions) {
  HashMap<std::uint32_t, std::uint32_t> block_ids;
  block_ids.Insert(0, 0);
  block_ids.Insert(1, 1);

  HashMap<std::uint32_t, AbstractMachineControlFlowGraph::Block> blocks;
  blocks.Insert(0, AbstractMachineControlFlowGraph::Block(0));
  blocks.Insert(1, AbstractMachineControlFlowGraph::Block(1));
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (auto* label = std::get_if<Label>(&instructions[i])) {
      block_ids.Insert(label->id, block_ids.size());
      std::size_t block_id = *block_ids.Find(label->id);
      blocks.Insert(block_id, AbstractMachineControlFlowGraph::Block(block_id));
    }
  }

  std::uint32_t current_block_id = 0;
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    if (auto* label = std::get_if<Label>(&instructions[i])) {
      current_block_id = *block_ids.Find(label->id);
    } else if (auto* jump = std::get_if<CondJump>(&instructions[i])) {
      std::uint32_t then_block_id = *block_ids.Find(jump->then_label);
      std::uint32_t else_block_id = *block_ids.Find(jump->else_label);

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
      std::uint32_t next_block_id = *block_ids.Find(jump->label);

      blocks.Find(current_block_id)
          ->next_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(next_block_id));
      blocks.Find(next_block_id)
          ->previous_blocks_.Insert(
              AbstractMachineControlFlowGraph::BlockRef(current_block_id));
    } else if (std::holds_alternative<Return>(instructions[i])) {
      blocks.Find(current_block_id)
          ->next_blocks_.Insert(AbstractMachineControlFlowGraph::BlockRef(1));
      blocks.Find(1)->previous_blocks_.Insert(
          AbstractMachineControlFlowGraph::BlockRef(current_block_id));
    }
  }

  return AbstractMachineControlFlowGraph(std::move(blocks));
}

}  // namespace lucid
