#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>

namespace lucid::arm64 {
namespace internal {

// Represents an ARM64 register.
class Reg {
 public:
  explicit Reg(std::uint8_t id) : id(id) { assert(id <= 0b11111); }

  std::uint8_t operator*() const { return id; }

 private:
  std::uint8_t id;
};

}  // namespace internal

// Represents a 32-bit ARM64 register.
class W : public internal::Reg {
 public:
  explicit W(std::uint8_t id) : Reg(id) {}
};

// Represents a 64-bit ARM64 register.
class X : public internal::Reg {
 public:
  explicit X(std::uint8_t id) : Reg(id) {}
};

// Represents an ARM64 immediate.
class Imm {
 public:
  explicit Imm(std::uint16_t value) : value(value) {
    assert(value <= 0b111111111111);
  }

  std::uint16_t operator*() const { return value; }

 private:
  std::uint16_t value;
};

// Represents an ARM64 instruction.
class Inst {
 public:
  explicit Inst(std::uint32_t value) : value(value) {
    assert(value <= 0b111111111111);
  }

  std::uint32_t operator*() const { return value; }

 private:
  std::uint32_t value;
};

// Builds a list of ARM64 instructions.
class Arm64 {
 public:
  // ADD <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  Inst Add(W rd, W rn, Imm imm, bool sh = false) {
    return Add(false, rd, rn, imm, sh);
  }

  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  Inst Add(X rd, X rn, Imm imm, bool sh = false) {
    return Add(true, rd, rn, imm, sh);
  }

  // ADR <Xd>, <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADR--Form-PC-relative-address-?lang=en
  Inst Adr(X rd, std::string_view label) {
    auto immlo = 0;  // TODO
    auto immhi = 0;  // TODO
    return Inst(0b00010000000000000000000000000000 | (immlo << 29) |
                (immhi << 5) | *rd);
  }

  // MOV <Wd>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  Inst Mov(W rd, W rm) { return Mov(false, rd, rm); }

  // MOV <Xd>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  Inst Mov(X rd, X rm) { return Mov(true, rd, rm); }

  // MOV <Wd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  Inst Mov(W rd, Imm imm) { return Mov(false, rd, imm); }

  // MOV <Xd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  Inst Mov(X rd, Imm imm) { return Mov(true, rd, imm); }

  // RET {<Xn>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
  Inst Ret(X rn = X(0)) {
    return Inst(0b11010110010111110000000000000000 | (*rn << 5));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    return StpPostIndex(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    return StpPostIndex(true, rt1, rt2, rn, imm);
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPreIndex(W rt1, W rt2, X rn, Imm imm) {
    return StpPreIndex(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPreIndex(X rt1, X rt2, X rn, Imm imm) {
    return StpPreIndex(true, rt1, rt2, rn, imm);
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpSignedOffset(W rt1, W rt2, X rn, Imm imm) {
    return StpSignedOffset(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpSignedOffset(X rt1, X rt2, X rn, Imm imm) {
    return StpSignedOffset(true, rt1, rt2, rn, imm);
  }

  // LDP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  Inst LdpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    return LdpPostIndex(false, rt1, rt2, rn, imm);
  }

  // LDP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  Inst LdpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    return LdpPostIndex(true, rt1, rt2, rn, imm);
  }

 private:
  // ADD (immediate)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  Inst Add(bool sf, internal::Reg rd, internal::Reg rn, Imm imm,
           bool sh = false) {
    return Inst(0b10010001000000000000000000000000 | (sf << 31) | (sh << 22) |
                (*imm << 10) | (*rn << 5) | *rd);
  }

  // MOV (register)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  Inst Mov(bool sf, internal::Reg rd, internal::Reg rm) {
    return Inst(0b00101010000000000000001111100000 | (sf << 31) | (*rm << 16) |
                *rd);
  }

  // MOV (wide immediate)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  Inst Mov(bool sf, internal::Reg rd, Imm imm) {
    return Inst(0b01010010100000000000000000000000 | (sf << 31) | (*imm << 5) |
                *rd);
  }

  // STP (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                    internal::Reg rn, Imm imm) {
    return Inst(0b00101000100000000000000000000000 | (opc << 31) |
                (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Pre-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpPreIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                   internal::Reg rn, Imm imm) {
    return Inst(0b00101001100000000000000000000000 | (opc << 31) |
                (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Signed offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  Inst StpSignedOffset(bool opc, internal::Reg rt1, internal::Reg rt2,
                       internal::Reg rn, Imm imm) {
    return Inst(0b00101001000000000000000000000000 | (opc << 31) |
                (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // LDP (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  Inst LdpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2, X rn,
                    Imm imm) {
    return Inst(0b00101000110000000000000000000000 | (opc << 30) |
                (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }
};

}  // namespace lucid::arm64
