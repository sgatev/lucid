#include "lucid/am/abi.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

// Takes `count` slots of the frame for arguments to stand in, and says where
// they begin.
SlotRange TakeArgSlots(AbstractMachineControlFlowGraph& am_cfg,
                       const CallingConvention& c, std::size_t count) {
  const SlotRange range = {
      .first = static_cast<std::uint32_t>(am_cfg.stack_slots.size()),
      .count = static_cast<std::uint32_t>(count),
  };
  for (std::size_t i = 0; i < count; ++i) {
    am_cfg.stack_slots.push_back(c.stack_arg_size);
  }
  return range;
}

// How many arguments `am_cfg` leaves on the stack for the widest call it
// makes.
std::size_t WidestStackCall(const AbstractMachineControlFlowGraph& am_cfg,
                            const CallingConvention& c) {
  std::size_t widest = 0;
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& inst : block.instructions) {
      const auto* call = std::get_if<FuncCall>(&inst);
      if (call == nullptr || call->args.size() <= c.max_register_args) continue;

      widest = std::max(widest, call->args.size() - c.max_register_args);
    }
  }
  return widest;
}

// Leaves every read of `reg` reading a register of its own, loaded from
// `slot` where the read stands.
//
// A value that is already in memory is one that is already spilt, and this
// is what spilling would have done to it, but for the store: nothing writes
// a parameter, so there is nothing to put away.
void LoadAtEveryUse(AbstractMachineControlFlowGraph& am_cfg, Reg reg,
                    std::uint32_t slot) {
  const auto load_into = [&](std::list<Instruction>& instructions,
                             std::list<Instruction>::iterator at, Reg& read) {
    read.id = am_cfg.next_free_reg_id++;
    instructions.insert(at, LoadStack{.offset = slot, .dst_reg = read});
  };

  for (auto& block : am_cfg.Blocks()) {
    for (auto i = block.instructions.begin(); i != block.instructions.end();
         ++i) {
      ForEachSourceRegister(*i, [&](Reg& read) {
        if (read == reg) load_into(block.instructions, i, read);
      });
    }

    // A branch reads what it decides on where its block ends, so the load
    // stands there, after everything the block does.
    if (auto& cond = block.branch_cond; cond.has_value() && *cond == reg) {
      load_into(block.instructions, block.instructions.end(), *cond);
    }
  }

  // A phi function reads its argument where control leaves the block that
  // argument comes from, so a load for it stands at the end of that block.
  // Every phi reading it out of the same block reads the one load.
  std::vector<std::optional<Reg>> loaded_leaving(am_cfg.Blocks().Size());
  for (auto& block : am_cfg.Blocks()) {
    for (auto& phi : block.phis) {
      for (std::size_t arg = 0; arg < phi.srcs.size(); ++arg) {
        if (phi.srcs[arg] != reg) continue;

        auto& loaded = loaded_leaving[block.preds[arg].id()];
        if (!loaded.has_value()) {
          Reg fresh = reg;
          fresh.id = am_cfg.next_free_reg_id++;
          am_cfg.GetBlock(block.preds[arg])
              .instructions.push_back(LoadStack{
                  .offset = slot,
                  .dst_reg = fresh,
              });
          loaded = fresh;
        }
        phi.srcs[arg] = *loaded;
      }
    }
  }
}

}  // namespace

FrameLayout LowerCallingConvention(AbstractMachineControlFlowGraph& am_cfg,
                                   const CallingConvention& convention) {
  FrameLayout layout;

  if (am_cfg.params.size() > convention.max_register_args) {
    const std::size_t on_stack =
        am_cfg.params.size() - convention.max_register_args;
    layout.incoming_args = TakeArgSlots(am_cfg, convention, on_stack);

    // Nothing holds a parameter that arrived in a slot, so every read of it
    // comes from there, and it is no longer one of the registers the
    // function is entered holding.
    for (std::size_t i = 0; i < on_stack; ++i) {
      LoadAtEveryUse(am_cfg, am_cfg.params[convention.max_register_args + i],
                     layout.incoming_args.first + i);
    }
    am_cfg.params.resize(convention.max_register_args);
  }

  const std::size_t widest = WidestStackCall(am_cfg, convention);
  if (widest == 0) return layout;

  // One run of slots for every call, because only one call is being made at
  // a time and each fills what it needs of them.
  layout.outgoing_args = TakeArgSlots(am_cfg, convention, widest);

  for (auto& block : am_cfg.Blocks()) {
    for (auto i = block.instructions.begin(); i != block.instructions.end();
         ++i) {
      auto* call = std::get_if<FuncCall>(&*i);
      if (call == nullptr ||
          call->args.size() <= convention.max_register_args) {
        continue;
      }

      // Put the arguments there is no register for where the call will look
      // for them, and take them off the call, so that nothing has to hold
      // them in a register while it is made. A call that read them all would
      // read more registers at once than there are.
      for (std::size_t at = convention.max_register_args;
           at < call->args.size(); ++at) {
        block.instructions.insert(
            i, StoreStack{
                   .offset = layout.outgoing_args.first +
                             (at - convention.max_register_args),
                   .src_reg = call->args[at].reg,
               });
      }
      call->args.resize(convention.max_register_args);
    }
  }
  return layout;
}

}  // namespace lucid
