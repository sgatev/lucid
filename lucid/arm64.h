#pragma once

#include <sys/ucred.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace lucid::arm64 {
namespace internal {

// Returns the two's complement of `n` interpreted as 7-bit signed integer.
inline std::uint16_t TwoCompl7(std::uint16_t n) {
  return (~n & 0b0000000001111111) + 1;
}

// Represents an ARM64 register.
class Reg {
 public:
  constexpr explicit Reg(std::uint8_t id) : id(id) { assert(id <= 0b11111); }

  std::uint8_t operator*() const { return id; }

 private:
  std::uint8_t id;
};

}  // namespace internal

// Represents a 32-bit ARM64 register.
class W : public internal::Reg {
 public:
  constexpr explicit W(std::uint8_t id) : Reg(id) {}
};

// Represents a 64-bit ARM64 register.
class X : public internal::Reg {
 public:
  constexpr explicit X(std::uint8_t id) : Reg(id) {}
};

// ARM64 stack pointer.
static constexpr X SP = X(0b11111);

// Represents an ARM64 immediate.
class Imm {
 public:
  explicit Imm(std::int16_t value) : value(value) {}

  std::int16_t operator*() const { return value; }

 private:
  std::int16_t value;
};

// Represents a basic ARM64 instruction.
class BasicInst {
 public:
  explicit BasicInst(std::uint32_t value) : value(value) {}

  std::uint32_t operator*() const { return value; }

  // Returns a binary representation of the instruction.
  std::uint32_t Encode(
      const std::unordered_map<std::string, std::size_t>& label_offsets) {
    return value;
  }

 private:
  std::uint32_t value;
};

// Represents an ARM64 ADR instruction.
class AdrInst {
 public:
  AdrInst(std::size_t pos, X rd, std::string_view label)
      : pos_(pos), rd_(rd), label_(label) {}

  // Returns a binary representation of the instruction.
  std::uint32_t Encode(
      const std::unordered_map<std::string, std::size_t>& label_offsets) {
    std::size_t label_offset = label_offsets.at(std::string(label_));
    std::size_t offset = (label_offset - pos_) * 4;
    auto immlo = offset & 0b11;
    auto immhi = (offset >> 2) & 0b1111111111111111111;
    return 0b00010000000000000000000000000000 | (immlo << 29) | (immhi << 5) |
           *rd_;
  }

 private:
  std::size_t pos_;
  X rd_;
  std::string label_;
};

// Represents an ARM64 B instruction.
class BInst {
 public:
  explicit BInst(std::size_t this_offset, std::string_view label)
      : this_offset_(this_offset), label_(label) {}

  // Returns a binary representation of the instruction.
  std::uint32_t Encode(
      const std::unordered_map<std::string, std::size_t>& label_offsets) {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    return 0b00010100000000000000000000000000 | offset;
  }

 private:
  std::size_t this_offset_;
  std::string label_;
};

// ARM64 condition.
enum class Cond : std::uint8_t {
  Eq = 0b0000,
};

// ARM64 inverse condition.
enum class InvCond : std::uint8_t {
  Ne = 0b0000,
  Eq = 0b0001,
  Lt = 0b1010,
  Gt = 0b1101,
};

// ARM64 extend.
enum class Extend : std::uint8_t {
  Uxtw = 0b010,
  Lsl = 0b011,
  Sxtw = 0b110,
  Sxtx = 0b111,
};

// Represents an ARM64 B.cond instruction.
class BCondInst {
 public:
  BCondInst(std::size_t this_offset, Cond cond, std::string_view label)
      : this_offset_(this_offset), cond_(cond), label_(label) {}

  // Returns a binary representation of the instruction.
  std::uint32_t Encode(
      const std::unordered_map<std::string, std::size_t>& label_offsets) {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    return 0b01010100000000000000000000000000 |
           (offset << 5) << static_cast<std::uint8_t>(cond_);
  }

 private:
  std::size_t this_offset_;
  Cond cond_;
  std::string label_;
};

// Represents an ARM64 BL instruction.
class BlInst {
 public:
  explicit BlInst(std::size_t this_offset, std::string_view label)
      : this_offset_(this_offset), label_(label) {}

  // Returns a binary representation of the instruction.
  std::uint32_t Encode(
      const std::unordered_map<std::string, std::size_t>& label_offsets) {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    return 0b10010100000000000000000000000000 | offset;
  }

 private:
  std::size_t this_offset_;
  std::string label_;
};

// Represents an ARM64 instruction.
using Inst = std::variant<BasicInst, AdrInst, BInst, BCondInst, BlInst>;

// Builds a list of ARM64 instructions.
class Arm64 {
 public:
  // Returns a binary representation of the assembled instructions.
  std::vector<std::uint32_t> Encode() const {
    std::vector<std::uint32_t> result;
    for (const Inst& inst : insts_) {
      result.push_back(std::visit(
          [this](auto inst) { return inst.Encode(label_offsets_); }, inst));
    }
    return result;
  }

  void Label(std::string label) { label_offsets_[label] = insts_.size(); }

  // ADD <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(W rd, W rn, Imm imm, bool sh = false) {
    insts_.push_back(Add(false, rd, rn, imm, sh));
  }

  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(X rd, X rn, Imm imm, bool sh = false) {
    insts_.push_back(Add(true, rd, rn, imm, sh));
  }

  // ADD <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(W rd, W rn, W rm) { insts_.push_back(Add(false, rd, rn, rm)); }

  // ADD <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(X rd, X rn, X rm) { insts_.push_back(Add(true, rd, rn, rm)); }

  // SUB <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(W rd, W rn, Imm imm, bool sh = false) {
    insts_.push_back(Sub(false, rd, rn, imm, sh));
  }

  // SUB <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(X rd, X rn, Imm imm, bool sh = false) {
    insts_.push_back(Sub(true, rd, rn, imm, sh));
  }

  // SUB <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(W rd, W rn, W rm) { insts_.push_back(Sub(false, rd, rn, rm)); }

  // SUB <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(X rd, X rn, X rm) { insts_.push_back(Sub(true, rd, rn, rm)); }

  // MUL <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(W rd, W rn, W rm) { insts_.push_back(Mul(false, rd, rn, rm)); }

  // MUL <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(X rd, X rn, X rm) { insts_.push_back(Mul(true, rd, rn, rm)); }

  // UDIV <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(W rd, W rn, W rm) { insts_.push_back(Udiv(false, rd, rn, rm)); }

  // UDIV <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(X rd, X rn, X rm) { insts_.push_back(Udiv(true, rd, rn, rm)); }

  // MSUB <Wd>, <Wn>, <Wm>, <Wa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(W rd, W rn, W rm, W ra) {
    insts_.push_back(Msub(false, rd, rn, rm, ra));
  }

  // MSUB <Xd>, <Xn>, <Xm>, <Xa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(X rd, X rn, X rm, X ra) {
    insts_.push_back(Msub(true, rd, rn, rm, ra));
  }

  // ADR <Xd>, <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADR--Form-PC-relative-address-?lang=en
  void Adr(X rd, std::string_view label) {
    insts_.push_back(AdrInst(insts_.size(), rd, label));
  }

  // MOV <Wd>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(W rd, W rm) { insts_.push_back(Mov(false, rd, rm)); }

  // MOV <Xd>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(X rd, X rm) { insts_.push_back(Mov(true, rd, rm)); }

  // MOV <Wd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(W rd, Imm imm) { insts_.push_back(Mov(false, rd, imm)); }

  // MOV <Xd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(X rd, Imm imm) { insts_.push_back(Mov(true, rd, imm)); }

  // RET {<Xn>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
  void Ret(X rn = X(30)) {
    insts_.push_back(
        BasicInst(0b11010110010111110000000000000000 | (*rn << 5)));
  }

  // SVC
  //
  // https://developer.arm.com/documentation/ddi0602/2024-09/Base-Instructions/SVC--Supervisor-call-?lang=en
  void Svc(Imm imm) {
    insts_.push_back(
        BasicInst(0b11010100000000000000000000000001 | (*imm << 5)));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    insts_.push_back(StpPostIndex(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    insts_.push_back(StpPostIndex(true, rt1, rt2, rn, imm));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(W rt1, W rt2, X rn, Imm imm) {
    insts_.push_back(StpPreIndex(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(X rt1, X rt2, X rn, Imm imm) {
    insts_.push_back(StpPreIndex(true, rt1, rt2, rn, imm));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(W rt1, W rt2, X rn, Imm imm) {
    insts_.push_back(StpSignedOffset(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(X rt1, X rt2, X rn, Imm imm) {
    insts_.push_back(StpSignedOffset(true, rt1, rt2, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(W rt, X rn, Imm imm) {
    insts_.push_back(StrPostIndex(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(X rt, X rn, Imm imm) {
    insts_.push_back(StrPostIndex(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(W rt, X rn, Imm imm) {
    insts_.push_back(StrPreIndex(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(X rt, X rn, Imm imm) {
    insts_.push_back(StrPreIndex(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(W rt, X rn, Imm imm) {
    insts_.push_back(StrUnsignedOffset(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(X rt, X rn, Imm imm) {
    insts_.push_back(StrUnsignedOffset(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(W rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    insts_.push_back(Str(false, rt, rn, rm, extend, amount));
  }

  // STR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(X rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    insts_.push_back(Str(false, rt, rn, rm, extend, amount));
  }

  // LDP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    insts_.push_back(LdpPostIndex(false, rt1, rt2, rn, imm));
  }

  // LDP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    insts_.push_back(LdpPostIndex(true, rt1, rt2, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(W rt, X rn, Imm imm) {
    insts_.push_back(LdrPreIndex(false, rt, rn, imm));
  }

  // LDR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(X rt, X rn, Imm imm) {
    insts_.push_back(LdrPreIndex(true, rt, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(W rt, X rn, Imm imm) {
    insts_.push_back(LdrUnsignedOffset(false, rt, rn, imm));
  }

  // LDR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(X rt, X rn, Imm imm) {
    insts_.push_back(LdrUnsignedOffset(true, rt, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(W rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    insts_.push_back(Ldr(false, rt, rn, rm, extend, amount));
  }

  // LDR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(X rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    insts_.push_back(Ldr(true, rt, rn, rm, extend, amount));
  }

  // B <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B--Branch-?lang=en
  void B(std::string_view label) {
    insts_.push_back(BInst(insts_.size(), label));
  }

  // B.cond <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B-cond--Branch-conditionally-?lang=en
  void B(Cond cond, std::string_view label) {
    insts_.push_back(BCondInst(insts_.size(), cond, label));
  }

  // BL <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/BL--Branch-with-link-?lang=en
  void Bl(std::string_view label) {
    insts_.push_back(BlInst(insts_.size(), label));
  }

  // CMP <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(W rn, Imm imm) { insts_.push_back(Cmp(false, rn, imm)); }

  // CMP <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(X rn, Imm imm) { insts_.push_back(Cmp(true, rn, imm)); }

  // CMP <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(W rn, W rm) { insts_.push_back(Cmp(false, rn, rm)); }

  // CMP <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(X rn, X rm) { insts_.push_back(Cmp(true, rn, rn)); }

  // CSET <Wd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(W rd, InvCond inv_cond) {
    insts_.push_back(Cset(false, rd, inv_cond));
  }

  // CSET <Xd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(X rd, InvCond inv_cond) {
    insts_.push_back(Cset(true, rd, inv_cond));
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
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b00101000100000000000000000000000 | (opc << 31) |
                     (imme << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Pre-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpPreIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                        internal::Reg rn, Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b00101001100000000000000000000000 | (opc << 31) |
                     (imme << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STP (Signed offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  BasicInst StpSignedOffset(bool opc, internal::Reg rt1, internal::Reg rt2,
                            internal::Reg rn, Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b00101001000000000000000000000000 | (opc << 31) |
                     (imme << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // STR (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  BasicInst StrPostIndex(bool opc, internal::Reg rt, internal::Reg rn,
                         Imm imm) {
    return BasicInst(0b10111000000000000000010000000000 | (opc << 30) |
                     (*imm << 12) | (*rn << 5) | *rt);
  }

  // STR (Pre-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  BasicInst StrPreIndex(bool opc, internal::Reg rt, internal::Reg rn, Imm imm) {
    return BasicInst(0b10111000000000000000110000000000 | (opc << 31) |
                     (*imm << 12) | (*rn << 5) | *rt);
  }

  // STR (Signed offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  BasicInst StrUnsignedOffset(bool opc, internal::Reg rt, internal::Reg rn,
                              Imm imm) {
    std::uint32_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;

    return BasicInst(0b10111001000000000000000000000000 | (opc << 31) |
                     (imme << 10) | (*rn << 5) | *rt);
  }

  // STR (register)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  BasicInst Str(bool opc, internal::Reg rt, X rn, internal::Reg rm,
                Extend extend, Imm amount) {
    return BasicInst(0b10111000001000000000100000000000 | (opc << 30) |
                     (*rm << 16) | (static_cast<std::uint8_t>(extend) << 13) |
                     (*amount << 12) | (*rn << 5) | *rt);
  }

  // LDP (Post-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  BasicInst LdpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2, X rn,
                         Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b00101000110000000000000000000000 | (opc << 31) |
                     (imme << 15) | (*rt2 << 10) | (*rn << 5) | *rt1);
  }

  // LDR (Pre-index)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  BasicInst LdrPreIndex(bool opc, internal::Reg rt, X rn, Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b10111000010000000000110000000000 | (opc << 30) |
                     (imme << 12) | (*rn << 5) | *rt);
  }

  // LDR (Unsigned offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  BasicInst LdrUnsignedOffset(bool opc, internal::Reg rt, X rn, Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = internal::TwoCompl7(-imme);

    return BasicInst(0b10111001010000000000000000000000 | (opc << 30) |
                     (imme << 10) | (*rn << 5) | *rt);
  }

  // LDR (register)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  BasicInst Ldr(bool opc, internal::Reg rt, X rn, internal::Reg rm,
                Extend extend, Imm amount) {
    return BasicInst(0b10111000011000000000100000000000 | (opc << 30) |
                     (*rm << 16) | (static_cast<std::uint8_t>(extend) << 13) |
                     (*amount << 12) | (*rn << 5) | *rt);
  }

  // CMP (immediate)
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  BasicInst Cmp(bool opc, internal::Reg rn, Imm imm) {
    return BasicInst(0b01110001000000000000000000011111 | (opc << 31) |
                     (*imm << 10) | (*rn << 5));
  }

  // CMP (shifted register)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  BasicInst Cmp(bool opc, internal::Reg rn, internal::Reg rm) {
    return BasicInst(0b01101011000000000000000000011111 | (opc << 31) |
                     (*rm << 16) | (*rn << 5));
  }

  // CSET
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  BasicInst Cset(bool opc, internal::Reg rd, InvCond inv_cond) {
    return BasicInst(0b00011010100111110000011111100000 | (opc << 31) |
                     (static_cast<std::uint8_t>(inv_cond) << 12) | *rd);
  }

  // ADD (shifted register
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  BasicInst Add(bool opc, internal::Reg rd, internal::Reg rn,
                internal::Reg rm) {
    return BasicInst(0b00001011000000000000000000000000 | (opc << 31) |
                     (*rm << 16) | (*rn << 5) | *rd);
  }

  // SUB (shifted register)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  BasicInst Sub(bool opc, internal::Reg rd, internal::Reg rn,
                internal::Reg rm) {
    return BasicInst(0b01001011000000000000000000000000 | (opc << 31) |
                     (*rm << 16) | (*rn << 5) | *rd);
  }

  // SUB (immediate)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  BasicInst Sub(bool sf, internal::Reg rd, internal::Reg rn, Imm imm,
                bool sh = false) {
    return BasicInst(0b01010001000000000000000000000000 | (sf << 31) |
                     (sh << 22) | (*imm << 10) | (*rn << 5) | *rd);
  }

  // MUL
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  BasicInst Mul(bool opc, internal::Reg rd, internal::Reg rn,
                internal::Reg rm) {
    return BasicInst(0b00011011000000000111110000000000 | (opc << 31) |
                     (*rm << 16) | (*rn << 5) | *rd);
  }

  // UDIV
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  BasicInst Udiv(bool opc, internal::Reg rd, internal::Reg rn,
                 internal::Reg rm) {
    return BasicInst(0b00011010110000000000100000000000 | (opc << 31) |
                     (*rm << 16) | (*rn << 5) | *rd);
  }

  // MSUB
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  BasicInst Msub(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm,
                 internal::Reg ra) {
    return BasicInst(0b00011011000000001000000000000000 | (opc << 31) |
                     (*rm << 16) | (*ra << 10) | (*rn << 5) | *rd);
  }

  std::unordered_map<std::string, std::size_t> label_offsets_;
  std::vector<Inst> insts_;
};

}  // namespace lucid::arm64
