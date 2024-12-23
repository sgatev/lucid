#pragma once

#include <sys/ucred.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
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

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    out.write(reinterpret_cast<const char*>(&value), 4);
  }

 private:
  std::uint32_t value;
};

// Represents an ARM64 ADR instruction.
class AdrInst {
 public:
  AdrInst(std::size_t pos, X rd, std::string_view label)
      : pos_(pos), rd_(rd), label_(label) {}

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    std::size_t label_offset = label_offsets.at(std::string(label_));
    std::size_t offset = (label_offset - pos_) * 4;
    auto immlo = offset & 0b11;
    auto immhi = (offset >> 2) & 0b1111111111111111111;
    std::uint32_t res = 0b00010000000000000000000000000000 | (immlo << 29) |
                        (immhi << 5) | *rd_;

    out.write(reinterpret_cast<const char*>(&res), 4);
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

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    std::uint32_t res = 0b00010100000000000000000000000000 | offset;

    out.write(reinterpret_cast<const char*>(&res), 4);
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

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    std::uint32_t res = 0b01010100000000000000000000000000 |
                        (offset << 5) << static_cast<std::uint8_t>(cond_);

    out.write(reinterpret_cast<const char*>(&res), 4);
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

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    std::size_t offset =
        (label_offsets.at(std::string(label_)) - this_offset_) &
        0b11111111111111111111111111;
    std::uint32_t res = 0b10010100000000000000000000000000 | offset;

    out.write(reinterpret_cast<const char*>(&res), 4);
  }

 private:
  std::size_t this_offset_;
  std::string label_;
};

class AscizInst {
 public:
  explicit AscizInst(std::string_view s) : s_(s) {}

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const {
    std::size_t count = 0;
    for (std::size_t i = 1; i < s_.size() - 1; ++i) {
      if (s_[i] == '\\' && s_[i + 1] == 'n') {
        ++count;
      } else {
        ++count;
      }
    }
    ++count;
    count += 4 - ((s_.size() - 2 + 1) % 4);
    return count;
  }

  // Writes the bytes produced by this instruction.
  void WriteBytes(
      const std::unordered_map<std::string, std::size_t>& label_offsets,
      std::ostream& out) const {
    for (std::size_t i = 1; i < s_.size() - 1; ++i) {
      if (s_[i] == '\\' && s_[i + 1] == 'n') {
        out.write(reinterpret_cast<const char*>("\n"), 1);
      } else {
        out.write(reinterpret_cast<const char*>(&s_[i]), 1);
      }
    }
    out.write(reinterpret_cast<const char*>("\0"), 1);

    std::size_t c = 4 - ((s_.size() - 2 + 1) % 4);
    for (std::size_t i = 0; i < c; ++i)
      out.write(reinterpret_cast<const char*>("\0"), 1);
  }

 private:
  std::string s_;
};

// Represents an ARM64 instruction.
using Inst =
    std::variant<BasicInst, AdrInst, BInst, BCondInst, BlInst, AscizInst>;

// Builds a list of ARM64 instructions.
class Arm64 {
 public:
  // Returns the number of bytes produced by the instructions.
  std::size_t OutputBytesCount() const { return output_bytes_count_; }

  // Writes the bytes produced by the instructions.
  void WriteBytes(std::ostream& out) const {
    for (const Inst& inst : insts_) {
      std::visit(
          [&](const auto& inst) { inst.WriteBytes(label_offsets_, out); },
          inst);
    }
  }

  void Label(std::string label) { label_offsets_[label] = insts_.size(); }

  // ADD <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(W rd, W rn, Imm imm, bool sh = false) {
    AddInst(Add(false, rd, rn, imm, sh));
  }

  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(X rd, X rn, Imm imm, bool sh = false) {
    AddInst(Add(true, rd, rn, imm, sh));
  }

  // ADD <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(W rd, W rn, W rm) { AddInst(Add(false, rd, rn, rm)); }

  // ADD <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(X rd, X rn, X rm) { AddInst(Add(true, rd, rn, rm)); }

  // SUB <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(W rd, W rn, Imm imm, bool sh = false) {
    AddInst(Sub(false, rd, rn, imm, sh));
  }

  // SUB <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(X rd, X rn, Imm imm, bool sh = false) {
    AddInst(Sub(true, rd, rn, imm, sh));
  }

  // SUB <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(W rd, W rn, W rm) { AddInst(Sub(false, rd, rn, rm)); }

  // SUB <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(X rd, X rn, X rm) { AddInst(Sub(true, rd, rn, rm)); }

  // MUL <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(W rd, W rn, W rm) { AddInst(Mul(false, rd, rn, rm)); }

  // MUL <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(X rd, X rn, X rm) { AddInst(Mul(true, rd, rn, rm)); }

  // UDIV <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(W rd, W rn, W rm) { AddInst(Udiv(false, rd, rn, rm)); }

  // UDIV <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(X rd, X rn, X rm) { AddInst(Udiv(true, rd, rn, rm)); }

  // MSUB <Wd>, <Wn>, <Wm>, <Wa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(W rd, W rn, W rm, W ra) { AddInst(Msub(false, rd, rn, rm, ra)); }

  // MSUB <Xd>, <Xn>, <Xm>, <Xa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(X rd, X rn, X rm, X ra) { AddInst(Msub(true, rd, rn, rm, ra)); }

  // ADR <Xd>, <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADR--Form-PC-relative-address-?lang=en
  void Adr(X rd, std::string_view label) {
    AddInst(AdrInst(insts_.size(), rd, label));
  }

  // MOV <Wd>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(W rd, W rm) { AddInst(Mov(false, rd, rm)); }

  // MOV <Xd>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(X rd, X rm) { AddInst(Mov(true, rd, rm)); }

  // MOV <Wd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(W rd, Imm imm) { AddInst(Mov(false, rd, imm)); }

  // MOV <Xd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(X rd, Imm imm) { AddInst(Mov(true, rd, imm)); }

  // RET {<Xn>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
  void Ret(X rn = X(30)) {
    AddInst(BasicInst(0b11010110010111110000000000000000 | (*rn << 5)));
  }

  // SVC
  //
  // https://developer.arm.com/documentation/ddi0602/2024-09/Base-Instructions/SVC--Supervisor-call-?lang=en
  void Svc(Imm imm) {
    AddInst(BasicInst(0b11010100000000000000000000000001 | (*imm << 5)));
  }

  void Asciz(std::string s) { AddInst(AscizInst(s)); }

  // STP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    AddInst(StpPostIndex(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    AddInst(StpPostIndex(true, rt1, rt2, rn, imm));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(W rt1, W rt2, X rn, Imm imm) {
    AddInst(StpPreIndex(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(X rt1, X rt2, X rn, Imm imm) {
    AddInst(StpPreIndex(true, rt1, rt2, rn, imm));
  }

  // STP <Wt1>, <Wt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(W rt1, W rt2, X rn, Imm imm) {
    AddInst(StpSignedOffset(false, rt1, rt2, rn, imm));
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(X rt1, X rt2, X rn, Imm imm) {
    AddInst(StpSignedOffset(true, rt1, rt2, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(W rt, X rn, Imm imm) {
    AddInst(StrPostIndex(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(X rt, X rn, Imm imm) {
    AddInst(StrPostIndex(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(W rt, X rn, Imm imm) {
    AddInst(StrPreIndex(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(X rt, X rn, Imm imm) {
    AddInst(StrPreIndex(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(W rt, X rn, Imm imm) {
    AddInst(StrUnsignedOffset(false, rt, rn, imm));
  }

  // STR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(X rt, X rn, Imm imm) {
    AddInst(StrUnsignedOffset(true, rt, rn, imm));
  }

  // STR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(W rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    AddInst(Str(false, rt, rn, rm, extend, amount));
  }

  // STR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(X rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    AddInst(Str(false, rt, rn, rm, extend, amount));
  }

  // LDP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    AddInst(LdpPostIndex(false, rt1, rt2, rn, imm));
  }

  // LDP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    AddInst(LdpPostIndex(true, rt1, rt2, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(W rt, X rn, Imm imm) {
    AddInst(LdrPreIndex(false, rt, rn, imm));
  }

  // LDR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(X rt, X rn, Imm imm) {
    AddInst(LdrPreIndex(true, rt, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(W rt, X rn, Imm imm) {
    AddInst(LdrUnsignedOffset(false, rt, rn, imm));
  }

  // LDR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(X rt, X rn, Imm imm) {
    AddInst(LdrUnsignedOffset(true, rt, rn, imm));
  }

  // LDR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(W rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    AddInst(Ldr(false, rt, rn, rm, extend, amount));
  }

  // LDR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(X rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    AddInst(Ldr(true, rt, rn, rm, extend, amount));
  }

  // B <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B--Branch-?lang=en
  void B(std::string_view label) { AddInst(BInst(insts_.size(), label)); }

  // B.cond <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B-cond--Branch-conditionally-?lang=en
  void B(Cond cond, std::string_view label) {
    AddInst(BCondInst(insts_.size(), cond, label));
  }

  // BL <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/BL--Branch-with-link-?lang=en
  void Bl(std::string_view label) { AddInst(BlInst(insts_.size(), label)); }

  // CMP <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(W rn, Imm imm) { AddInst(Cmp(false, rn, imm)); }

  // CMP <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(X rn, Imm imm) { AddInst(Cmp(true, rn, imm)); }

  // CMP <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(W rn, W rm) { AddInst(Cmp(false, rn, rm)); }

  // CMP <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(X rn, X rm) { AddInst(Cmp(true, rn, rm)); }

  // CSET <Wd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(W rd, InvCond inv_cond) { AddInst(Cset(false, rd, inv_cond)); }

  // CSET <Xd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(X rd, InvCond inv_cond) { AddInst(Cset(true, rd, inv_cond)); }

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
    return BasicInst(0b10111000000000000000110000000000 | (opc << 30) |
                     (*imm << 12) | (*rn << 5) | *rt);
  }

  // STR (Signed offset)
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  BasicInst StrUnsignedOffset(bool opc, internal::Reg rt, internal::Reg rn,
                              Imm imm) {
    std::int16_t imme = *imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;

    return BasicInst(0b10111001000000000000000000000000 | (opc << 30) |
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

  void AddInst(Inst inst) {
    output_bytes_count_ += std::visit(
        [](const auto& inst) { return inst.OutputBytesCount(); }, inst);
    insts_.push_back(std::move(inst));
  }

  std::unordered_map<std::string, std::size_t> label_offsets_;
  std::vector<Inst> insts_;
  std::size_t output_bytes_count_ = 0;
};

}  // namespace lucid::arm64
