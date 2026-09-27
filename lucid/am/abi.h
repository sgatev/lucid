#pragma once

#include <cstddef>
#include <cstdint>

#include "lucid/am/cfg.h"

namespace lucid {

// Where a call leaves what it is passing, which is a matter for the machine
// the code is compiled for rather than for the abstract machine.
struct CallingConvention {
  // How many arguments a call passes in registers. The ones past these it
  // leaves on the stack, for the called function to read where it reads
  // them.
  //
  // Every parameter is live where a function is entered, so one that arrives
  // in a register holds a colour there. This can therefore be no more than
  // the number of colours there are to hand out.
  std::size_t max_register_args;

  // How much room an argument left on the stack takes, whatever its width,
  // so that the one after it stands this much further along. A narrower
  // value is written into the low bytes of its own room and read back from
  // there.
  int stack_arg_size;
};

// A run of a function's stack slots, given by where it begins and how many
// slots it holds.
struct SlotRange {
  std::uint32_t first = 0;
  std::uint32_t count = 0;

  bool Holds(std::size_t slot) const {
    return slot >= first && slot - first < count;
  }
};

// Where the arguments a function was handed and the arguments it hands on
// stand among its stack slots, which is what lowering a calling convention
// settles and what laying out a frame needs to know.
struct FrameLayout {
  // The slots the caller left the parameters it had no register to pass. They
  // stand above this function's frame rather than in it, because the caller
  // wrote them before the frame existed.
  SlotRange incoming_args;

  // The slots this function leaves the arguments it has no register to pass.
  // They stand at the foot of the frame, which is where the stack pointer is
  // when a call is made and so where the called function looks for them. As
  // many as the widest call the function makes needs.
  SlotRange outgoing_args;
};

// Rewrites `am_cfg` to pass and to take its arguments the way `convention`
// says, and returns where that leaves them.
//
// Until this has run the graph says nothing about registers: a function has
// its parameters and a call has its arguments, however many there are of
// either. After it, the ones there is no register for stand in slots of the
// frame, which is where the machine leaves them.
//
// A parameter left in a slot is read from there wherever it is read, and is
// no longer among the registers the function is entered holding. Nothing in
// the graph names it afterwards, so nothing downstream has to know that a
// parameter was ever anything but a register.
FrameLayout LowerCallingConvention(AbstractMachineControlFlowGraph& am_cfg,
                                   const CallingConvention& convention);

}  // namespace lucid
