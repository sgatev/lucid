#pragma once

#include <sys/ucred.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>

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

// Represents a basic ARM64 instruction.
class BasicInst {
 public:
  explicit BasicInst(std::uint32_t value) : value(value) {
    assert(value <= 0b111111111111);
  }

  std::uint32_t operator*() const { return value; }

  std::uint32_t Encode(std::size_t pos) { return value; }

 private:
  std::uint32_t value;
};

// Represents an ARM64 ADR instruction.
class AdrInst {
 public:
  AdrInst(X rd, std::size_t label_offset)
      : rd_(rd), label_offset_(label_offset) {}

  std::uint32_t Encode(std::size_t pos) {
    std::size_t offset = (label_offset_ - pos) * 4;
    auto immlo = offset & 0b11;
    auto immhi = (offset >> 2) & 0b1111111111111111111;
    return 0b00010000000000000000000000000000 | (immlo << 29) | (immhi << 5) |
           *rd_;
  }

 private:
  X rd_;
  std::size_t label_offset_;
};

// Represents an ARM64 instruction.
using Inst = std::variant<BasicInst, AdrInst>;

// Encodes a stream of ARM64 instructions in binary.
class Encoder {
 public:
  std::uint32_t Encode(Inst inst) {
    return std::visit([this](auto inst) { return inst.Encode(pos_++); }, inst);
  }

 private:
  std::size_t pos_ = 0;
};

// Builds a list of ARM64 instructions.
class Arm64 {
 public:
  // ADD <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  BasicInst Add(W rd, W rn, Imm imm, bool sh = false) {
    return Add(false, rd, rn, imm, sh);
  }

  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  BasicInst Add(X rd, X rn, Imm imm, bool sh = false) {
    return Add(true, rd, rn, imm, sh);
  }

  // ADR <Xd>, <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADR--Form-PC-relative-address-?lang=en
  AdrInst Adr(X rd, std::string_view label) {
    return AdrInst(rd, label_offsets_[label]);
  }

  // MOV <Wd>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  BasicInst Mov(W rd, W rm) { return Mov(false, rd, rm); }

  // MOV <Xd>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  BasicInst Mov(X rd, X rm) { return Mov(true, rd, rm); }

  // MOV <Wd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  BasicInst Mov(W rd, Imm imm) { return Mov(false, rd, imm); }

  // MOV <Xd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  BasicInst Mov(X rd, Imm imm) { return Mov(true, rd, imm); }

  // RET {<Xn>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
  BasicInst Ret(X rn = X(0)) {
    return BasicInst(0b11010110010111110000000000000000 | (*rn << 5));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    return StpPostIndex(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    return StpPostIndex(true, rt1, rt2, rn, imm);
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPreIndex(W rt1, W rt2, X rn, Imm imm) {
    return StpPreIndex(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPreIndex(X rt1, X rt2, X rn, Imm imm) {
    return StpPreIndex(true, rt1, rt2, rn, imm);
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpSignedOffset(W rt1, W rt2, X rn, Imm imm) {
    return StpSignedOffset(false, rt1, rt2, rn, imm);
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpSignedOffset(X rt1, X rt2, X rn, Imm imm) {
    return StpSignedOffset(true, rt1, rt2, rn, imm);
  }

  // LDP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  BasicInst LdpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    return LdpPostIndex(false, rt1, rt2, rn, imm);
  }

  // LDP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  BasicInst LdpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    return LdpPostIndex(true, rt1, rt2, rn, imm);
  }

  // B <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B--Branch-?lang=en
  BasicInst B(std::string_view label) {
    std::size_t offset = label_offsets_[label];
    return BasicInst(0b00010100000000000000000000000000 | offset);
  }

  // BL <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/BL--Branch-with-link-?lang=en
  BasicInst Bl(std::string_view label) {
    std::size_t offset = label_offsets_[label];
    return BasicInst(0b10010100000000000000000000000000 | offset);
  }

 private:
  // ADD (immediate)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  BasicInst Add(bool sf, internal::Reg rd, internal::Reg rn, Imm imm,
                bool sh = false) {
    return BasicInst(0b10010001000000000000000000000000 | (sf << 31) |
                     (sh << 22) | (*imm << 10) | (*rn << 5) | *rd);
  }

  // MOV (register)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  BasicInst Mov(bool sf, internal::Reg rd, internal::Reg rm) {
    return BasicInst(0b00101010000000000000001111100000 | (sf << 31) |
                     (*rm << 16) | *rd);
  }

  // MOV (wide immediate)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  BasicInst Mov(bool sf, internal::Reg rd, Imm imm) {
    return BasicInst(0b01010010100000000000000000000000 | (sf << 31) |
                     (*imm << 5) | *rd);
  }

  // STP (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                         internal::Reg rn, Imm imm) {
    return BasicInst(0b00101000100000000000000000000000 | (opc << 31) |
                     (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Pre-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPreIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                        internal::Reg rn, Imm imm) {
    return BasicInst(0b00101001100000000000000000000000 | (opc << 31) |
                     (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Signed offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpSignedOffset(bool opc, internal::Reg rt1, internal::Reg rt2,
                            internal::Reg rn, Imm imm) {
    return BasicInst(0b00101001000000000000000000000000 | (opc << 31) |
                     (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // LDP (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  BasicInst LdpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2, X rn,
                         Imm imm) {
    return BasicInst(0b00101000110000000000000000000000 | (opc << 30) |
                     (*imm << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  std::unordered_map<std::string_view, std::size_t> label_offsets_;
};

}  // namespace lucid::arm64
