#pragma once

#include <cassert>
#include <cstdint>

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

namespace internal {

// ADD (immediate)
//
// https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/ADD--immediate---Add--immediate--?lang=en
Inst Add(bool sf, Reg rd, Reg rn, Imm imm, bool sh = false) {
  return Inst(0b10010001000000000000000000000000 | (sf << 31) | (sh << 22) |
              (*imm << 10) | (*rn << 5) | *rd);
}

// MOV (register)
//
// https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
Inst Mov(bool sf, Reg rd, Reg rm) {
  return Inst(0b00101010000000000000001111100000 | (sf << 31) | (*rm << 16) |
              *rd);
}

}  // namespace internal

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

// MOV <Wd>, <Wm>
//
// https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
Inst Mov(W rd, W rm) { return Mov(false, rd, rm); }

// MOV <Xd>, <Xm>
//
// https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/MOV--register---Move--register---an-alias-of-ORR--shifted-register--?lang=en
Inst Mov(X rd, X rm) { return Mov(true, rd, rm); }

// RET {<Xn>}
//
// https://developer.arm.com/documentation/ddi0602/2022-09/Base-Instructions/RET--Return-from-subroutine-?lang=en
Inst Ret(X rn = X(0)) {
  return Inst(0b11010110010111110000000000000000 | (*rn << 5));
}

}  // namespace lucid::arm64
