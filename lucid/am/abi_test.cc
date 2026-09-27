#include "lucid/am/abi.h"

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

// Two registers to pass arguments in, which is few enough that a function
// running out of them takes only a few parameters to write down.
constexpr CallingConvention kConvention = {
    .max_register_args = 2,
    .stack_arg_size = 8,
};

Reg R(std::int32_t id) { return Reg(id, RegSize32); }

TEST(Test, LowerCallingConventionLeavesWhatFitsInRegistersAlone) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddParam(R(1));
  builder.AddParam(R(2));
  builder.AddInstruction(block, AddReg{
                                    .res_reg = R(3),
                                    .lhs_reg = R(1),
                                    .rhs_reg = R(2),
                                });

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  const FrameLayout layout = LowerCallingConvention(am_cfg, kConvention);

  EXPECT_EQ(layout.incoming_args.count, 0);
  EXPECT_EQ(layout.outgoing_args.count, 0);
  EXPECT_TRUE(am_cfg.stack_slots.empty());
  EXPECT_THAT(am_cfg.params, ElementsEqual(R(1), R(2)));
  EXPECT_THAT(am_cfg.GetBlock(block).instructions, ElementsEqual(AddReg{
                                                       .res_reg = R(3),
                                                       .lhs_reg = R(1),
                                                       .rhs_reg = R(2),
                                                   }));
}

TEST(Test, LowerCallingConventionReadsAParameterOnTheStackWhereItIsRead) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddParam(R(1));
  builder.AddParam(R(2));
  builder.AddParam(R(3));
  // The third parameter is read twice, and each read is its own.
  builder.AddInstruction(block, AddReg{
                                    .res_reg = R(4),
                                    .lhs_reg = R(3),
                                    .rhs_reg = R(3),
                                });

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  const FrameLayout layout = LowerCallingConvention(am_cfg, kConvention);

  // The third is no longer one of the registers the function is entered
  // holding, because nothing hands it one.
  EXPECT_THAT(am_cfg.params, ElementsEqual(R(1), R(2)));

  EXPECT_EQ(layout.incoming_args.first, 0);
  EXPECT_EQ(layout.incoming_args.count, 1);
  EXPECT_THAT(am_cfg.stack_slots, ElementsEqual(8));

  EXPECT_THAT(am_cfg.GetBlock(block).instructions, ElementsEqual(
                                                       LoadStack{
                                                           .offset = 0,
                                                           .dst_reg = R(5),
                                                       },
                                                       LoadStack{
                                                           .offset = 0,
                                                           .dst_reg = R(6),
                                                       },
                                                       AddReg{
                                                           .res_reg = R(4),
                                                           .lhs_reg = R(5),
                                                           .rhs_reg = R(6),
                                                       }));
}

TEST(Test, LowerCallingConventionLeavesTheArgumentsPastTheRegistersOnTheStack) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, FuncCall{
                                    .label = "f",
                                    .args = {{R(1)}, {R(2)}, {R(3)}, {R(4)}},
                                    .res = FuncCall::Slot{R(5)},
                                });

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  const FrameLayout layout = LowerCallingConvention(am_cfg, kConvention);

  EXPECT_EQ(layout.outgoing_args.first, 0);
  EXPECT_EQ(layout.outgoing_args.count, 2);
  EXPECT_THAT(am_cfg.stack_slots, ElementsEqual(8, 8));

  // The call carries what it passes in registers, and what it does not stands
  // in the slots before it.
  EXPECT_THAT(am_cfg.GetBlock(block).instructions,
              ElementsEqual(
                  StoreStack{
                      .offset = 0,
                      .src_reg = R(3),
                  },
                  StoreStack{
                      .offset = 1,
                      .src_reg = R(4),
                  },
                  FuncCall{
                      .label = "f",
                      .args = {{R(1)}, {R(2)}},
                      .res = FuncCall::Slot{R(5)},
                  }));
}

TEST(Test, LowerCallingConventionSharesTheRoomForArgumentsBetweenCalls) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, FuncCall{
                                    .label = "narrow",
                                    .args = {{R(1)}, {R(2)}, {R(3)}},
                                });
  builder.AddInstruction(block,
                         FuncCall{
                             .label = "wide",
                             .args = {{R(1)}, {R(2)}, {R(3)}, {R(4)}, {R(5)}},
                         });

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  const FrameLayout layout = LowerCallingConvention(am_cfg, kConvention);

  // Room for the widest of them, which the other one writes the front of:
  // only one call is being made at a time.
  EXPECT_EQ(layout.outgoing_args.count, 3);
  EXPECT_THAT(am_cfg.stack_slots, ElementsEqual(8, 8, 8));
  EXPECT_THAT(am_cfg.GetBlock(block).instructions,
              ElementsEqual(
                  StoreStack{
                      .offset = 0,
                      .src_reg = R(3),
                  },
                  FuncCall{
                      .label = "narrow",
                      .args = {{R(1)}, {R(2)}},
                  },
                  StoreStack{
                      .offset = 0,
                      .src_reg = R(3),
                  },
                  StoreStack{
                      .offset = 1,
                      .src_reg = R(4),
                  },
                  StoreStack{
                      .offset = 2,
                      .src_reg = R(5),
                  },
                  FuncCall{
                      .label = "wide",
                      .args = {{R(1)}, {R(2)}},
                  }));
}

TEST(Test, LowerCallingConventionTakesTheRoomForArgumentsAfterTheOwnSlots) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddParam(R(1));
  builder.AddParam(R(2));
  builder.AddParam(R(3));
  builder.AddInstruction(block, FuncCall{
                                    .label = "f",
                                    .args = {{R(1)}, {R(2)}, {R(3)}},
                                });

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  // A slot the function already had, which the argument slots stand after
  // rather than over.
  am_cfg.stack_slots.push_back(4);
  const auto slots_before = am_cfg.stack_slots.size();
  const FrameLayout layout = LowerCallingConvention(am_cfg, kConvention);

  EXPECT_EQ(layout.incoming_args.first, slots_before);
  EXPECT_EQ(layout.outgoing_args.first, slots_before + 1);
  EXPECT_THAT(am_cfg.stack_slots, ElementsEqual(4, 8, 8));
}

TEST(Test, LowerCallingConventionLoadsAParameterOnTheStackForAPhiFunction) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto entry = builder.AddBlock();
  auto body = builder.AddBlock();
  builder.SetFirst(entry);
  builder.SetLast(body);
  builder.AddEdge(entry, body);
  builder.AddParam(R(1));
  builder.AddParam(R(2));
  builder.AddParam(R(3));
  builder.AddPhi(body, {.dst = R(4), .srcs = {R(3)}});

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  LowerCallingConvention(am_cfg, kConvention);

  // A phi reads its argument where control leaves the block it comes from,
  // so the load stands at the end of that block rather than in this one.
  EXPECT_THAT(am_cfg.GetBlock(entry).instructions, ElementsEqual(LoadStack{
                                                       .offset = 0,
                                                       .dst_reg = R(5),
                                                   }));
  EXPECT_THAT(am_cfg.GetBlock(body).phis[0].srcs, ElementsEqual(R(5)));
}

}  // namespace
}  // namespace lucid
