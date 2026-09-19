#include "lucid/arm64/assembler.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid::arm64 {
namespace {

// Returns the instruction words that `emit` assembles to.
//
// The expected values below are written as those words rather than as bytes,
// because that is how an encoding is given in the ARM reference and how a
// disassembler reads one back: `0x11000c41` is `add w1, w2, #3`. Each is the
// encoding the assembler and a disassembler agree on, so a wrong bit here is
// a wrong instruction in a compiled program.
template <typename F>
std::vector<std::uint32_t> Encode(F emit) {
  Assembler assembler;
  emit(assembler);

  std::ostringstream out;
  assembler.WriteBytes(out);
  const std::string bytes = out.str();

  std::vector<std::uint32_t> words(bytes.size() / sizeof(std::uint32_t));
  std::memcpy(words.data(), bytes.data(), words.size() * sizeof(std::uint32_t));
  return words;
}

TEST(Test, InstSize) { EXPECT_EQ(sizeof(Inst), 48); }

TEST(Test, AddEncodesBothRegisterWidths) {
  // add w1, w2, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Add(W(1), W(2), Imm(3)); }),
              ElementsEqual(0x11000c41u));
  // add x1, x2, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Add(X(1), X(2), Imm(3)); }),
              ElementsEqual(0x91000c41u));
  // add x1, x2, #3, lsl #12
  EXPECT_THAT(Encode([](Assembler& a) { a.Add(X(1), X(2), Imm(3), true); }),
              ElementsEqual(0x91400c41u));
  // add w1, w2, w3
  EXPECT_THAT(Encode([](Assembler& a) { a.Add(W(1), W(2), W(3)); }),
              ElementsEqual(0x0b030041u));
  // add x1, x2, x3
  EXPECT_THAT(Encode([](Assembler& a) { a.Add(X(1), X(2), X(3)); }),
              ElementsEqual(0x8b030041u));
}

TEST(Test, SubEncodesBothRegisterWidths) {
  // sub w1, w2, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Sub(W(1), W(2), Imm(3)); }),
              ElementsEqual(0x51000c41u));
  // sub x1, x2, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Sub(X(1), X(2), Imm(3)); }),
              ElementsEqual(0xd1000c41u));
  // sub w1, w2, w3
  EXPECT_THAT(Encode([](Assembler& a) { a.Sub(W(1), W(2), W(3)); }),
              ElementsEqual(0x4b030041u));
  // sub x1, x2, x3
  EXPECT_THAT(Encode([](Assembler& a) { a.Sub(X(1), X(2), X(3)); }),
              ElementsEqual(0xcb030041u));
}

TEST(Test, MultiplicationAndDivisionEncode) {
  // mul w1, w2, w3
  EXPECT_THAT(Encode([](Assembler& a) { a.Mul(W(1), W(2), W(3)); }),
              ElementsEqual(0x1b037c41u));
  // mul x1, x2, x3
  EXPECT_THAT(Encode([](Assembler& a) { a.Mul(X(1), X(2), X(3)); }),
              ElementsEqual(0x9b037c41u));
  // udiv w1, w2, w3
  EXPECT_THAT(Encode([](Assembler& a) { a.Udiv(W(1), W(2), W(3)); }),
              ElementsEqual(0x1ac30841u));
  // udiv x1, x2, x3
  EXPECT_THAT(Encode([](Assembler& a) { a.Udiv(X(1), X(2), X(3)); }),
              ElementsEqual(0x9ac30841u));
  // msub w1, w2, w3, w4
  EXPECT_THAT(Encode([](Assembler& a) { a.Msub(W(1), W(2), W(3), W(4)); }),
              ElementsEqual(0x1b039041u));
  // msub x1, x2, x3, x4
  EXPECT_THAT(Encode([](Assembler& a) { a.Msub(X(1), X(2), X(3), X(4)); }),
              ElementsEqual(0x9b039041u));
}

TEST(Test, MovEncodesRegistersImmediatesAndTheStackPointer) {
  // mov w1, w2
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(W(1), W(2)); }),
              ElementsEqual(0x2a0203e1u));
  // mov x1, x2
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(X(1), X(2)); }),
              ElementsEqual(0xaa0203e1u));
  // mov w1, #5
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(W(1), Imm(5)); }),
              ElementsEqual(0x528000a1u));
  // mov x1, #5
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(X(1), Imm(5)); }),
              ElementsEqual(0xd28000a1u));
  // mov x1, sp
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(X(1), SP); }),
              ElementsEqual(0x910003e1u));
  // mov sp, x1
  EXPECT_THAT(Encode([](Assembler& a) { a.Mov(SP, X(1)); }),
              ElementsEqual(0x9100003fu));
}

TEST(Test, CmpEncodesImmediatesAndRegisters) {
  // cmp w1, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Cmp(W(1), Imm(3)); }),
              ElementsEqual(0x71000c3fu));
  // cmp x1, #3
  EXPECT_THAT(Encode([](Assembler& a) { a.Cmp(X(1), Imm(3)); }),
              ElementsEqual(0xf1000c3fu));
  // cmp w1, w2
  EXPECT_THAT(Encode([](Assembler& a) { a.Cmp(W(1), W(2)); }),
              ElementsEqual(0x6b02003fu));
  // cmp x1, x2
  EXPECT_THAT(Encode([](Assembler& a) { a.Cmp(X(1), X(2)); }),
              ElementsEqual(0xeb02003fu));
}

TEST(Test, CsetEncodesTheCondition) {
  // cset w1, ne
  EXPECT_THAT(Encode([](Assembler& a) { a.Cset(W(1), InvCond::Ne); }),
              ElementsEqual(0x1a9f07e1u));
  // cset w1, eq
  EXPECT_THAT(Encode([](Assembler& a) { a.Cset(W(1), InvCond::Eq); }),
              ElementsEqual(0x1a9f17e1u));
  // cset w1, lt
  EXPECT_THAT(Encode([](Assembler& a) { a.Cset(W(1), InvCond::Lt); }),
              ElementsEqual(0x1a9fa7e1u));
  // cset x1, gt
  EXPECT_THAT(Encode([](Assembler& a) { a.Cset(X(1), InvCond::Gt); }),
              ElementsEqual(0x9a9fd7e1u));
}

TEST(Test, ReturnAndSupervisorCallEncode) {
  // ret
  EXPECT_THAT(Encode([](Assembler& a) { a.Ret(); }),
              ElementsEqual(0xd65f03c0u));
  // ret x1
  EXPECT_THAT(Encode([](Assembler& a) { a.Ret(X(1)); }),
              ElementsEqual(0xd65f0020u));
  // svc #0
  EXPECT_THAT(Encode([](Assembler& a) { a.Svc(Imm(0)); }),
              ElementsEqual(0xd4000001u));
}

TEST(Test, RegisterPairsEncodeTheirIndexing) {
  // stp x29, x30, [sp, #-16]!
  EXPECT_THAT(
      Encode([](Assembler& a) { a.StpPreIndex(X(29), X(30), SP, Imm(-16)); }),
      ElementsEqual(0xa9bf7bfdu));
  // stp x1, x2, [sp], #16
  EXPECT_THAT(
      Encode([](Assembler& a) { a.StpPostIndex(X(1), X(2), SP, Imm(16)); }),
      ElementsEqual(0xa8810be1u));
  // stp w1, w2, [sp, #8]
  EXPECT_THAT(
      Encode([](Assembler& a) { a.StpSignedOffset(W(1), W(2), SP, Imm(8)); }),
      ElementsEqual(0x29010be1u));
  // ldp x29, x30, [sp], #16
  EXPECT_THAT(
      Encode([](Assembler& a) { a.LdpPostIndex(X(29), X(30), SP, Imm(16)); }),
      ElementsEqual(0xa8c17bfdu));
}

TEST(Test, LoadsAndStoresEncodeTheirIndexing) {
  // The pre- and post-index offsets are counts of bytes, and the unsigned
  // offset is a count of accesses, so the same 16 encodes differently.
  //
  // str x1, [sp, #8]!
  EXPECT_THAT(Encode([](Assembler& a) { a.StrPreIndex(X(1), SP, Imm(8)); }),
              ElementsEqual(0xf8008fe1u));
  // str w1, [sp, #-4]!
  EXPECT_THAT(Encode([](Assembler& a) { a.StrPreIndex(W(1), SP, Imm(-4)); }),
              ElementsEqual(0xb81fcfe1u));
  // str w1, [sp], #4
  EXPECT_THAT(Encode([](Assembler& a) { a.StrPostIndex(W(1), SP, Imm(4)); }),
              ElementsEqual(0xb80047e1u));
  // str x1, [sp, #16]
  EXPECT_THAT(
      Encode([](Assembler& a) { a.StrUnsignedOffset(X(1), SP, Imm(16)); }),
      ElementsEqual(0xf9000be1u));
  // ldr x1, [sp, #8]!
  EXPECT_THAT(Encode([](Assembler& a) { a.LdrPreIndex(X(1), SP, Imm(8)); }),
              ElementsEqual(0xf8408fe1u));
  // ldr x1, [sp, #-8]!
  EXPECT_THAT(Encode([](Assembler& a) { a.LdrPreIndex(X(1), SP, Imm(-8)); }),
              ElementsEqual(0xf85f8fe1u));
  // ldr x1, [sp, #16]
  EXPECT_THAT(
      Encode([](Assembler& a) { a.LdrUnsignedOffset(X(1), SP, Imm(16)); }),
      ElementsEqual(0xf9400be1u));
}

TEST(Test, LoadsAndStoresEncodeRegisterOffsets) {
  // str w1, [x2, w3, uxtw #2]
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Str(W(1), X(2), W(3), Extend::Uxtw, Imm(1));
              }),
              ElementsEqual(0xb8235841u));
  // ldr x1, [x2, x3, lsl #3]
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Ldr(X(1), X(2), X(3), Extend::Lsl, Imm(1));
              }),
              ElementsEqual(0xf8637841u));
}

TEST(Test, BranchesEncodeTheDistanceToTheirLabel) {
  // ret ; b #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.B("l");
              }),
              ElementsEqual(0xd65f03c0u, 0x17ffffffu));
  // b #8 ; ret
  EXPECT_THAT(Encode([](Assembler& a) {
                a.B("l");
                a.Ret();
                a.Label("l");
              }),
              ElementsEqual(0x14000002u, 0xd65f03c0u));
  // ret ; bl #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.Bl("l");
              }),
              ElementsEqual(0xd65f03c0u, 0x97ffffffu));
}

TEST(Test, ConditionalBranchesEncodeTheirConditionAndDistance) {
  // ret ; b.eq #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.B(Cond::Eq, "l");
              }),
              ElementsEqual(0xd65f03c0u, 0x54ffffe0u));
  // b.eq #8 ; ret
  EXPECT_THAT(Encode([](Assembler& a) {
                a.B(Cond::Eq, "l");
                a.Ret();
                a.Label("l");
              }),
              ElementsEqual(0x54000040u, 0xd65f03c0u));
  // `Cond` declares only `Eq`, which is zero, so only a second condition can
  // show that the condition reaches the encoding at all. `0b1011` is the
  // condition ARM calls `lt`.
  //
  // ret ; b.lt #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.B(static_cast<Cond>(0b1011), "l");
              }),
              ElementsEqual(0xd65f03c0u, 0x54ffffebu));
}

TEST(Test, AddressAndLiteralLoadsEncodeTheDistanceToTheirLabel) {
  // ret ; adr x1, #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.Adr(X(1), "l");
              }),
              ElementsEqual(0xd65f03c0u, 0x10ffffe1u));
  // A literal load reads as many bytes as its register holds, so the width
  // belongs in the encoding.
  //
  // ret ; ldr x1, #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.Ldr(X(1), "l");
              }),
              ElementsEqual(0xd65f03c0u, 0x58ffffe1u));
  // ret ; ldr w1, #-4
  EXPECT_THAT(Encode([](Assembler& a) {
                a.Label("l");
                a.Ret();
                a.Ldr(W(1), "l");
              }),
              ElementsEqual(0xd65f03c0u, 0x18ffffe1u));
}

TEST(Test, DataIsWrittenAsItIsGiven) {
  // A string arrives as it was written in source, quote marks and all, and is
  // terminated and padded to a four byte boundary.
  Assembler assembler;
  assembler.Asciz(R"("ab")");
  assembler.Long(7);

  EXPECT_EQ(assembler.OutputBytesCount(), 12u);

  std::ostringstream out;
  assembler.WriteBytes(out);
  EXPECT_EQ(out.str(), std::string("ab\0\0\7\0\0\0\0\0\0\0", 12));
}

TEST(Test, LabelsRecordWhereTheyWereInserted) {
  Assembler assembler;
  assembler.Ret();
  assembler.Global("global");
  assembler.Ret();
  assembler.Bl(assembler.External("external"));

  EXPECT_THAT(assembler.GlobalLabels().Get("global"), Optional(Equals(4)));
  EXPECT_TRUE(assembler.ExternalLabels().contains("external"));
  EXPECT_EQ(assembler.OutputBytesCount(), 12u);
}

}  // namespace
}  // namespace lucid::arm64
