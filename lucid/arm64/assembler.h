#pragma once

#include <sys/ucred.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/numeric/bits.h"
#include "lucid/core/string/encoding.h"

namespace lucid::arm64 {
namespace internal {

// Represents an ARM64 register.
class Reg {
 public:
  constexpr explicit Reg(std::uint8_t id) : id_(id) { assert(id <= 0b11111); }

  // Returns the ID of the register.
  operator std::uint8_t() const { return id_; }

 private:
  std::uint8_t id_;
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

// Represents a 64-bit ARM64 stack pointer.
class SP : public X {
 public:
  constexpr explicit SP() : X(0b11111) {}
};

// ARM64 stack pointer.
static constexpr SP SP;

// An external label.
class ExternalLabel {
 public:
  ExternalLabel() = default;

  constexpr explicit ExternalLabel(std::string label)
      : label_(std::move(label)) {}

  // Returns the value of the label.
  operator std::string() const { return label_; }

 private:
  std::string label_;
};

// Represents an ARM64 immediate.
class Imm {
 public:
  explicit Imm(std::int16_t value) : value_(value) {}

  // Returns the value of the immediate.
  operator std::uint16_t() const { return value_; }

 private:
  std::int16_t value_;
};

// Represents a literal ARM64 instruction.
class LitInst {
 public:
  explicit LitInst(std::uint32_t value) : value_(value) {}

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    out.write(reinterpret_cast<const char*>(&value_), 4);
  }

 private:
  std::uint32_t value_;
};

// Represents an ARM64 ADR instruction.
class AdrInst {
 public:
  AdrInst(std::size_t pos, X rd, std::string_view label)
      : pos_(pos), rd_(rd), label_(label) {}

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const { return 4; }

  // Writes the bytes produced by this instruction.
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    auto label_offset = label_offsets.Find(std::string(label_));
    assert(label_offset.has_value());
    std::size_t offset = *label_offset - pos_;
    auto immlo = offset & 0b11;
    auto immhi = (offset >> 2) & 0b1111111111111111111;
    std::uint32_t res =
        0b00010000000000000000000000000000 | immlo << 29 | immhi << 5 | rd_;

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
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    auto label_offset = label_offsets.Find(std::string(label_));
    assert(label_offset.has_value());
    std::size_t offset =
        ((*label_offset - this_offset_) / 4) & 0b11111111111111111111111111;
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
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    auto label_offset = label_offsets.Find(std::string(label_));
    assert(label_offset.has_value());
    std::size_t offset =
        ((*label_offset - this_offset_) / 4) & 0b11111111111111111111111111;
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
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    std::size_t offset = 0;
    if (!label_.empty()) {
      auto label_offset = label_offsets.Find(std::string(label_));
      assert(label_offset.has_value());
      offset =
          ((*label_offset - this_offset_) / 4) & 0b11111111111111111111111111;
    }
    std::uint32_t res = 0b10010100000000000000000000000000 | offset;

    out.write(reinterpret_cast<const char*>(&res), 4);
  }

 private:
  std::size_t this_offset_;
  std::string label_;
};

class AscizInst {
 public:
  explicit AscizInst(std::string_view s) : s_(s) {
    // Remove quote marks at the beginning and end of the string.
    s_.remove_prefix(1);
    s_.remove_suffix(1);
  }

  // Returns the number of bytes produced by this instruction.
  std::size_t OutputBytesCount() const {
    const std::size_t length = EncodedStringLength(s_);
    return length + (4 - (length % 4));
  }

  // Writes the bytes produced by this instruction.
  void WriteBytes(const HashMap<std::string, std::size_t>& label_offsets,
                  std::ostream& out) const {
    const std::size_t length = WriteEncodedString(s_, out);
    std::size_t remainder = 4 - (length % 4);
    for (; remainder > 0; --remainder) out.put(0);
  }

 private:
  std::string_view s_;
};

// Represents an ARM64 instruction.
using Inst =
    std::variant<LitInst, AdrInst, BInst, BCondInst, BlInst, AscizInst>;

// Builds a list of ARM64 instructions.
class Assembler {
 public:
  // Returns the number of bytes produced by the instructions.
  std::size_t OutputBytesCount() const { return insts_size_; }

  // Writes the bytes produced by the instructions.
  void WriteBytes(std::ostream& out) const {
    for (const Inst& inst : insts_) {
      std::visit(
          [&](const auto& inst) { inst.WriteBytes(label_offsets_, out); },
          inst);
    }
  }

  // Returns all global labels that were inserted.
  const HashMap<std::string, std::size_t>& GlobalLabels() const {
    return global_label_offsets_;
  }

  // Returns all external labels that were created.
  const std::map<std::string, std::vector<std::size_t>>& ExternalLabels()
      const {
    return external_labels_;
  }

  // Inserts local `label` after the last instruction that was added.
  void Label(std::string label) {
    label_offsets_.Set(std::move(label), insts_size_);
  }

  // Inserts global `label` after the last instruction that was added.
  void Global(std::string label) {
    global_label_offsets_.Set(std::move(label), insts_size_);
  }

  // Creates an external label.
  ExternalLabel External(std::string label) {
    external_labels_.try_emplace(label);
    return ExternalLabel(std::move(label));
  }

  // Inserts ADD (immediate) instruction.
  //
  // ADD <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(W rd, W rn, Imm imm, bool sh = false) {
    Insert(Add(false, rd, rn, imm, sh));
  }

  // Inserts ADD (immediate) instruction.
  //
  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
  void Add(X rd, X rn, Imm imm, bool sh = false) {
    Insert(Add(true, rd, rn, imm, sh));
  }

  // Inserts ADD (shifted register) instruction.
  //
  // ADD <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(W rd, W rn, W rm) { Insert(Add(false, rd, rn, rm)); }

  // Inserts ADD (shifted register) instruction.
  //
  // ADD <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADD--shifted-register---Add--shifted-register--?lang=en
  void Add(X rd, X rn, X rm) { Insert(Add(true, rd, rn, rm)); }

  // Inserts SUB (immediate) instruction.
  //
  // SUB <Wd|WSP>, <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(W rd, W rn, Imm imm, bool sh = false) {
    Insert(Sub(false, rd, rn, imm, sh));
  }

  // Inserts SUB (immediate) instruction.
  //
  // SUB <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--immediate---Subtract--immediate--?lang=en
  void Sub(X rd, X rn, Imm imm, bool sh = false) {
    Insert(Sub(true, rd, rn, imm, sh));
  }

  // Inserts SUB (shifted register) instruction.
  //
  // SUB <Wd>, <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(W rd, W rn, W rm) { Insert(Sub(false, rd, rn, rm)); }

  // Inserts SUB (shifted register) instruction.
  //
  // SUB <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/SUB--shifted-register---Subtract--shifted-register--?lang=en
  void Sub(X rd, X rn, X rm) { Insert(Sub(true, rd, rn, rm)); }

  // Inserts MUL instruction.
  //
  // MUL <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(W rd, W rn, W rm) { Insert(Mul(false, rd, rn, rm)); }

  // Inserts MUL instruction.
  //
  // MUL <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MUL--Multiply--an-alias-of-MADD-?lang=en
  void Mul(X rd, X rn, X rm) { Insert(Mul(true, rd, rn, rm)); }

  // Inserts UDIV instruction.
  //
  // UDIV <Wd>, <Wn>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(W rd, W rn, W rm) { Insert(Udiv(false, rd, rn, rm)); }

  // Inserts UDIV instruction.
  //
  // UDIV <Xd>, <Xn>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/UDIV--Unsigned-divide-?lang=en
  void Udiv(X rd, X rn, X rm) { Insert(Udiv(true, rd, rn, rm)); }

  // Inserts MSUB instruction.
  //
  // MSUB <Wd>, <Wn>, <Wm>, <Wa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(W rd, W rn, W rm, W ra) { Insert(Msub(false, rd, rn, rm, ra)); }

  // Inserts MSUB instruction.
  //
  // MSUB <Xd>, <Xn>, <Xm>, <Xa>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/MSUB--Multiply-subtract-?lang=en
  void Msub(X rd, X rn, X rm, X ra) { Insert(Msub(true, rd, rn, rm, ra)); }

  // Inserts ADR instruction.
  //
  // ADR <Xd>, <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/ADR--Form-PC-relative-address-?lang=en
  void Adr(X rd, std::string_view label) {
    Insert(AdrInst(insts_size_, rd, label));
  }

  // Inserts MOV (register) instruction.
  //
  // MOV <Wd>, <Wm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(W rd, W rm) { Insert(Mov(false, rd, rm)); }

  // Inserts MOV (register) instruction.
  //
  // MOV <Xd>, <Xm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
  void Mov(X rd, X rm) { Insert(Mov(true, rd, rm)); }

  // Insert MOV (wide immediate) instruction.
  //
  // MOV <Wd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(W rd, Imm imm) { Insert(Mov(false, rd, imm)); }

  // Insert MOV (wide immediate) instruction.
  //
  // MOV <Xd>, #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--wide-immediate---Move--wide-immediate---an-alias-of-MOVZ-?lang=en
  void Mov(X rd, Imm imm) { Insert(Mov(true, rd, imm)); }

  // Insert MOV (to/from SP) instruction.
  //
  // MOV <Xd|SP>, <Xn|SP>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--to-from-SP---Move-between-register-and-stack-pointer--an-alias-of-ADD--immediate--?lang=en
  void Mov(X rd, class SP rn) { Insert(MovSP(true, rd, rn)); }

  // Insert MOV (to/from SP) instruction.
  //
  // MOV <Xd|SP>, <Xn|SP>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--to-from-SP---Move-between-register-and-stack-pointer--an-alias-of-ADD--immediate--?lang=en
  void Mov(class SP rd, X rn) { Insert(MovSP(true, rd, rn)); }

  // Insert RET instruction.
  //
  // RET {<Xn>}
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
  void Ret(X rn = X(30)) {
    Insert(LitInst(0b11010110010111110000000000000000 | rn << 5));
  }

  // Insert SVC instruction.
  //
  // SVC
  //
  // https://developer.arm.com/documentation/ddi0602/2024-09/Base-Instructions/SVC--Supervisor-call-?lang=en
  void Svc(Imm imm) {
    Insert(LitInst(0b11010100000000000000000000000001 | imm << 5));
  }

  // Insert a null-terminated string.
  void Asciz(std::string_view s) { Insert(AscizInst(s)); }

  // Insert STP post-index instruction.
  //
  // STP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    Insert(StpPostIndex(false, rt1, rt2, rn, imm));
  }

  // Insert STP post-index instruction.
  //
  // STP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    Insert(StpPostIndex(true, rt1, rt2, rn, imm));
  }

  // Insert STP pre-index instruction.
  //
  // STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(W rt1, W rt2, X rn, Imm imm) {
    Insert(StpPreIndex(false, rt1, rt2, rn, imm));
  }

  // Insert STP pre-index instruction.
  //
  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpPreIndex(X rt1, X rt2, X rn, Imm imm) {
    Insert(StpPreIndex(true, rt1, rt2, rn, imm));
  }

  // Insert STP signed offset instruction.
  //
  // STP <Wt1>, <Wt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(W rt1, W rt2, X rn, Imm imm) {
    Insert(StpSignedOffset(false, rt1, rt2, rn, imm));
  }

  // Insert STP signed offset instruction.
  //
  // STP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/STP--Store-Pair-of-Registers-?lang=en#iclass_post_indexed
  void StpSignedOffset(X rt1, X rt2, X rn, Imm imm) {
    Insert(StpSignedOffset(true, rt1, rt2, rn, imm));
  }

  // Insert STR (immediate) post-index instruction.
  //
  // STR <Wt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(W rt, X rn, Imm imm) {
    Insert(StrPostIndex(false, rt, rn, imm));
  }

  // Insert STR (immediate) post-index instruction.
  //
  // STR <Xt>, [<Xn|SP>], #<simm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPostIndex(X rt, X rn, Imm imm) {
    Insert(StrPostIndex(true, rt, rn, imm));
  }

  // Insert STR (immediate) pre-index instruction.
  //
  // STR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(W rt, X rn, Imm imm) {
    Insert(StrPreIndex(false, rt, rn, imm));
  }

  // Insert STR (immediate) pre-index instruction.
  //
  // STR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrPreIndex(X rt, X rn, Imm imm) {
    Insert(StrPreIndex(true, rt, rn, imm));
  }

  // Insert STR (immediate) unsigned offset instruction.
  //
  // STR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(W rt, X rn, Imm imm) {
    Insert(StrUnsignedOffset(false, rt, rn, imm));
  }

  // Insert STR (immediate) unsigned offset instruction.
  //
  // STR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--immediate---Store-register--immediate--?lang=en
  void StrUnsignedOffset(X rt, X rn, Imm imm) {
    Insert(StrUnsignedOffset(true, rt, rn, imm));
  }

  // Insert STR (register) instruction.
  //
  // STR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(W rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    Insert(Str(false, rt, rn, rm, extend, amount));
  }

  // Insert STR (register) instruction.
  //
  // STR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/STR--register---Store-register--register--?lang=en
  void Str(X rt, X rn, internal::Reg rm, Extend extend, Imm amount) {
    Insert(Str(false, rt, rn, rm, extend, amount));
  }

  // Insert LDP post-index instruction.
  //
  // LDP <Wt1>, <Wt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(W rt1, W rt2, X rn, Imm imm) {
    Insert(LdpPostIndex(false, rt1, rt2, rn, imm));
  }

  // Insert LDP post-index instruction.
  //
  // LDP <Xt1>, <Xt2>, [<Xn|SP>], #<imm>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDP--Load-pair-of-registers-?lang=en
  void LdpPostIndex(X rt1, X rt2, X rn, Imm imm) {
    Insert(LdpPostIndex(true, rt1, rt2, rn, imm));
  }

  // Insert LDP pre-index instruction.
  //
  // LDR <Wt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(W rt, X rn, Imm imm) {
    Insert(LdrPreIndex(false, rt, rn, imm));
  }

  // Insert LDP pre-index instruction.
  //
  // LDR <Xt>, [<Xn|SP>, #<simm>]!
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrPreIndex(X rt, X rn, Imm imm) {
    Insert(LdrPreIndex(true, rt, rn, imm));
  }

  // Insert LDP signed offset instruction.
  //
  // LDR <Wt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(W rt, X rn, Imm imm) {
    Insert(LdrUnsignedOffset(false, rt, rn, imm));
  }

  // Insert LDP signed offset instruction.
  //
  // LDR <Xt>, [<Xn|SP>{, #<pimm>}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--immediate---Load-register--immediate--?lang=en
  void LdrUnsignedOffset(X rt, X rn, Imm imm) {
    Insert(LdrUnsignedOffset(true, rt, rn, imm));
  }

  // Insert LDR (register) instruction.
  //
  // LDR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(W rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    Insert(Ldr(false, rt, rn, rm, extend, amount));
  }

  // Insert LDR (register) instruction.
  //
  // LDR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/LDR--register---Load-register--register--?lang=en
  void Ldr(X rt, X rn, internal::Reg rm, Extend extend, Imm amount = Imm(0)) {
    Insert(Ldr(true, rt, rn, rm, extend, amount));
  }

  // Insert B instruction.
  //
  // B <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B--Branch-?lang=en
  void B(std::string_view label) { Insert(BInst(insts_size_, label)); }

  // Insert B.cond instruction.
  //
  // B.cond <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/B-cond--Branch-conditionally-?lang=en
  void B(Cond cond, std::string_view label) {
    Insert(BCondInst(insts_size_, cond, label));
  }

  // Insert BL instruction.
  //
  // BL <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/BL--Branch-with-link-?lang=en
  void Bl(std::string_view label) { Insert(BlInst(insts_size_, label)); }

  // Insert BL instruction.
  //
  // BL <label>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/BL--Branch-with-link-?lang=en
  void Bl(ExternalLabel label) {
    external_labels_[label].push_back(insts_size_);
    Bl("");
  }

  // Insert CMP (immediate) instruction.
  //
  // CMP <Wn|WSP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(W rn, Imm imm) { Insert(Cmp(false, rn, imm)); }

  // Insert CMP (immediate) instruction.
  //
  // CMP <Xn|SP>, #<imm>{, <shift>}
  //
  // https://developer.arm.com/documentation/100076/0100/A64-Instruction-Set-Reference/A64-General-Instructions/CMP--immediate-
  void Cmp(X rn, Imm imm) { Insert(Cmp(true, rn, imm)); }

  // Insert CMP (shifted register) instruction.
  //
  // CMP <Wn>, <Wm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(W rn, W rm) { Insert(Cmp(false, rn, rm)); }

  // Insert CMP (shifted register) instruction.
  //
  // CMP <Xn>, <Xm>{, <shift> #<amount>}
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CMP--shifted-register---Compare--shifted-register---an-alias-of-SUBS--shifted-register--?lang=en
  void Cmp(X rn, X rm) { Insert(Cmp(true, rn, rm)); }

  // Insert CSET instruction.
  //
  // CSET <Wd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(W rd, InvCond inv_cond) { Insert(Cset(false, rd, inv_cond)); }

  // Insert CSET instruction.
  //
  // CSET <Xd>, <invcond>
  //
  // https://developer.arm.com/documentation/ddi0602/2024-06/Base-Instructions/CSET--Conditional-set--an-alias-of-CSINC-?lang=en
  void Cset(X rd, InvCond inv_cond) { Insert(Cset(true, rd, inv_cond)); }

 private:
  LitInst Add(bool sf, internal::Reg rd, internal::Reg rn, Imm imm,
              bool sh = false) {
    return LitInst(0b10010001000000000000000000000000 | sf << 31 | sh << 22 |
                   imm << 10 | rn << 5 | rd);
  }

  LitInst Mov(bool sf, internal::Reg rd, internal::Reg rm) {
    return LitInst(0b00101010000000000000001111100000 | sf << 31 | rm << 16 |
                   rd);
  }

  LitInst Mov(bool sf, internal::Reg rd, Imm imm) {
    return LitInst(0b01010010100000000000000000000000 | sf << 31 | imm << 5 |
                   rd);
  }

  LitInst MovSP(bool sf, internal::Reg rd, internal::Reg rn) {
    return LitInst(0b00010001000000000000000000000000 | sf << 31 | rn << 5 |
                   rd);
  }

  LitInst StpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                       internal::Reg rn, Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b00101000100000000000000000000000 | opc << 31 | imme << 15 |
                   rt2 << 10 | rn << 5 | rt1);
  }

  LitInst StpPreIndex(bool opc, internal::Reg rt1, internal::Reg rt2,
                      internal::Reg rn, Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b00101001100000000000000000000000 | opc << 31 | imme << 15 |
                   rt2 << 10 | rn << 5 | rt1);
  }

  LitInst StpSignedOffset(bool opc, internal::Reg rt1, internal::Reg rt2,
                          internal::Reg rn, Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b00101001000000000000000000000000 | opc << 31 | imme << 15 |
                   rt2 << 10 | rn << 5 | rt1);
  }

  LitInst StrPostIndex(bool opc, internal::Reg rt, internal::Reg rn, Imm imm) {
    return LitInst(0b10111000000000000000010000000000 | opc << 30 | imm << 12 |
                   rn << 5 | rt);
  }

  LitInst StrPreIndex(bool opc, internal::Reg rt, internal::Reg rn, Imm imm) {
    return LitInst(0b10111000000000000000110000000000 | (opc << 30) |
                   imm << 12 | rn << 5 | rt);
  }

  LitInst StrUnsignedOffset(bool opc, internal::Reg rt, internal::Reg rn,
                            Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;

    return LitInst(0b10111001000000000000000000000000 | opc << 30 | imme << 10 |
                   rn << 5 | rt);
  }

  LitInst Str(bool opc, internal::Reg rt, X rn, internal::Reg rm, Extend extend,
              Imm amount) {
    return LitInst(0b10111000001000000000100000000000 | opc << 30 | rm << 16 |
                   static_cast<std::uint8_t>(extend) << 13 | amount << 12 |
                   rn << 5 | rt);
  }

  LitInst LdpPostIndex(bool opc, internal::Reg rt1, internal::Reg rt2, X rn,
                       Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b00101000110000000000000000000000 | opc << 31 | imme << 15 |
                   rt2 << 10 | rn << 5 | rt1);
  }

  LitInst LdrPreIndex(bool opc, internal::Reg rt, X rn, Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b10111000010000000000110000000000 | opc << 30 | imme << 12 |
                   rn << 5 | rt);
  }

  LitInst LdrUnsignedOffset(bool opc, internal::Reg rt, X rn, Imm imm) {
    std::int16_t imme = imm;
    if (opc)
      imme /= 8;
    else
      imme /= 4;
    if (imme < 0) imme = TwosComplement7(-imme);

    return LitInst(0b10111001010000000000000000000000 | opc << 30 | imme << 10 |
                   rn << 5 | rt);
  }

  LitInst Ldr(bool opc, internal::Reg rt, X rn, internal::Reg rm, Extend extend,
              Imm amount) {
    return LitInst(0b10111000011000000000100000000000 | opc << 30 | rm << 16 |
                   static_cast<std::uint8_t>(extend) << 13 | amount << 12 |
                   rn << 5 | rt);
  }

  LitInst Cmp(bool opc, internal::Reg rn, Imm imm) {
    return LitInst(0b01110001000000000000000000011111 | opc << 31 | imm << 10 |
                   rn << 5);
  }

  LitInst Cmp(bool opc, internal::Reg rn, internal::Reg rm) {
    return LitInst(0b01101011000000000000000000011111 | opc << 31 | rm << 16 |
                   rn << 5);
  }

  LitInst Cset(bool opc, internal::Reg rd, InvCond inv_cond) {
    return LitInst(0b00011010100111110000011111100000 | opc << 31 |
                   static_cast<std::uint8_t>(inv_cond) << 12 | rd);
  }

  LitInst Add(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm) {
    return LitInst(0b00001011000000000000000000000000 | opc << 31 | rm << 16 |
                   rn << 5 | rd);
  }

  LitInst Sub(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm) {
    return LitInst(0b01001011000000000000000000000000 | opc << 31 | rm << 16 |
                   rn << 5 | rd);
  }

  LitInst Sub(bool sf, internal::Reg rd, internal::Reg rn, Imm imm,
              bool sh = false) {
    return LitInst(0b01010001000000000000000000000000 | sf << 31 | sh << 22 |
                   imm << 10 | rn << 5 | rd);
  }

  LitInst Mul(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm) {
    return LitInst(0b00011011000000000111110000000000 | opc << 31 | rm << 16 |
                   rn << 5 | rd);
  }

  LitInst Udiv(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm) {
    return LitInst(0b00011010110000000000100000000000 | opc << 31 | rm << 16 |
                   rn << 5 | rd);
  }

  LitInst Msub(bool opc, internal::Reg rd, internal::Reg rn, internal::Reg rm,
               internal::Reg ra) {
    return LitInst(0b00011011000000001000000000000000 | opc << 31 | rm << 16 |
                   ra << 10 | rn << 5 | rd);
  }

  void Insert(Inst inst) {
    insts_size_ += std::visit(
        [](const auto& inst) { return inst.OutputBytesCount(); }, inst);
    insts_.push_back(std::move(inst));
  }

  HashMap<std::string, std::size_t> label_offsets_;
  HashMap<std::string, std::size_t> global_label_offsets_;
  std::vector<Inst> insts_;
  std::size_t insts_size_ = 0;
  std::map<std::string, std::vector<std::size_t>> external_labels_;
};

}  // namespace lucid::arm64
