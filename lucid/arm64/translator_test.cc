#include "lucid/arm64/translator.h"

#include <sstream>
#include <string>
#include <vector>

#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Returns the machine code that `assembler` produced.
std::string Bytes(const arm64::Assembler& assembler) {
  std::ostringstream out;
  assembler.WriteBytes(out);
  return out.str();
}

TEST(Test, GenerateArmStartBinaryWorks) {
  arm64::Assembler assembler;

  GenerateArmStartBinary(assembler);

  // The entry point is the one symbol the linker has to resolve, and the
  // sequence calls into libc, which the linker resolves for it.
  EXPECT_THAT(assembler.GlobalLabels().Get("_start"), Optional(Equals(0)));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_printf"));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_nanosleep"));

  // The sequence calls the program's entry function, so it cannot be written
  // until something defines it.
  assembler.Label("main");
  EXPECT_EQ(Bytes(assembler).size(), assembler.OutputBytesCount());
  EXPECT_EQ(assembler.OutputBytesCount() % 4, 0u);
}

TEST(Test, GenerateArmEndBinaryWritesStringsInPoolOrder) {
  // A string literal is interned as it was written, quote marks included.
  SyntaxContext syn_ctx;
  StringIndex::Ref ab = syn_ctx.AddIdent(R"("ab")");
  StringIndex::Ref cd = syn_ctx.AddIdent(R"("cd")");

  // The pool order is the order the strings were first referenced, which is
  // what their `strN` labels are numbered by, so it is the order they must be
  // written in -- not the order they were interned in.
  AbstractMachineState am_state;
  am_state.strings = {cd, ab};

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  // Each string is terminated and padded to a four byte boundary.
  EXPECT_EQ(Bytes(assembler), std::string("cd\0\0ab\0\0", 8));
}

TEST(Test, GenerateArmEndBinaryWritesIntegers) {
  SyntaxContext syn_ctx;
  AbstractMachineState am_state;
  am_state.ints.Insert(7);

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  EXPECT_EQ(Bytes(assembler), std::string("\x07\0\0\0\0\0\0\0", 8));
}

TEST(Test, GenerateArmEndBinaryWritesNothingWithoutConstants) {
  SyntaxContext syn_ctx;
  AbstractMachineState am_state;

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  EXPECT_EQ(assembler.OutputBytesCount(), 0u);
}

TEST(Test, GenerateArmAssemblyBinaryWorks) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, Return{.res_reg = {1, RegSize32}});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("main", {}, am_cfg, assembler);

  const std::string bytes = Bytes(assembler);
  EXPECT_EQ(bytes.size() % 4, 0u);
  // A function ends by returning to its caller.
  EXPECT_EQ(bytes.substr(bytes.size() - 4), std::string("\xc0\x03\x5f\xd6", 4));
}

}  // namespace
}  // namespace lucid
