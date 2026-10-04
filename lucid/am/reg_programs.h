#pragma once

#include <format>
#include <string>

namespace lucid {

// Programs the register allocator is tested and measured on, each built to
// crowd it in a way of its own. They are written as source, so that what the
// allocator is given is what the compiler in front of it would give it.

// A function of `count` values, each one feeding only the next.
//
// A value dies where the one after it is born, so few are ever live at once:
// what grows with `count` is the number of registers rather than how many
// are live together.
inline std::string ChainedValues(int count) {
  std::string code = "fun main(): Int32 {\n  val v0: Int32 = 1\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val v{}: Int32 = v{} + {}\n", i, i - 1, i);
  }
  code += std::format("  return v{}\n}}\n", count - 1);
  return code;
}

// A function of `count` values that are all live at once.
//
// Every value is read after the last one is written, so each interferes with
// every other: the graph is dense, and with only ten registers to colour it
// with, the allocator has to spill.
inline std::string LiveValues(int count) {
  std::string code = "fun main(): Int32 {\n";
  for (int i = 0; i < count; ++i) {
    code += std::format("  val v{}: Int32 = {}\n", i, i);
  }
  code += "  val sum: Int32 = v0\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val sum{}: Int32 = sum + v{}\n", i, i);
  }
  code += std::format("  return sum{}\n}}\n", count - 1);
  return code;
}

// A function of `count` parameters, each of them read. Every parameter is
// live where the function is entered, because that is where the caller leaves
// it, so the ones there is no register for arrive on the stack.
inline std::string ManyParameters(int count) {
  std::string code = "fun main(";
  for (int i = 0; i < count; ++i) {
    code += std::format("{}p{}: Int32", i == 0 ? "" : ", ", i);
  }
  code += "): Int32 {\n  val sum0: Int32 = p0\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val sum{}: Int32 = sum{} + p{}\n", i, i - 1, i);
  }
  code += std::format("  return sum{}\n}}\n", count - 1);
  return code;
}

// `crossing` values written on one side of a branch and all read after it,
// so that a phi function waits for each of them where the sides meet, and
// `inside` more live only within that side, to crowd the registers there.
//
// What crosses the branch is what is read furthest ahead, so it is what the
// spilling takes, which is to say a register a phi function reads.
inline std::string BranchingValues(int crossing, int inside) {
  std::string code = "fun main(): Int32 {\n  val a: Int32 = 1\n";
  for (int i = 0; i < crossing; ++i) {
    code += std::format("  mut val v{}: Int32 = {}\n", i, i + 1);
  }

  code += "  if a == 1 {\n";
  for (int i = 0; i < inside; ++i) {
    code += std::format("    val w{}: Int32 = {}\n", i, i + 1);
  }
  if (inside > 0) {
    code += "    val t0: Int32 = w0\n";
    for (int i = 1; i < inside; ++i) {
      code += std::format("    val t{}: Int32 = t{} + w{}\n", i, i - 1, i);
    }
    for (int i = 0; i < crossing; ++i) {
      code += std::format("    mut v{} = v{} + t{}\n", i, i, inside - 1);
    }
  }
  code += "  } else {\n";
  for (int i = 0; i < crossing; ++i) {
    code += std::format("    mut v{} = v{} - 1\n", i, i);
  }
  code += "  }\n";

  code += "  val sum0: Int32 = v0\n";
  for (int i = 1; i < crossing; ++i) {
    code += std::format("  val sum{}: Int32 = sum{} + v{}\n", i, i - 1, i);
  }
  code += std::format("  return sum{}\n}}\n", crossing - 1);
  return code;
}

// `count` branches one after another, each writing the same `crossing` values
// on both of its sides, with all of them read after the last.
//
// Every value has a phi function at every place the sides meet, so most of
// the function is joins: it is what an analysis joins over the most, and what
// spilling is slowest on.
inline std::string Diamonds(int count, int crossing) {
  std::string code = "fun main(): Int32 {\n  val a: Int32 = 1\n";
  for (int i = 0; i < crossing; ++i) {
    code += std::format("  mut val v{}: Int32 = {}\n", i, i + 1);
  }
  for (int k = 0; k < count; ++k) {
    code += std::format("  if a == {} {{\n", k % 3);
    for (int i = 0; i < crossing; ++i) {
      code += std::format("    mut v{} = v{} + 1\n", i, i);
    }
    code += "  } else {\n";
    for (int i = 0; i < crossing; ++i) {
      code += std::format("    mut v{} = v{} - 1\n", i, i);
    }
    code += "  }\n";
  }
  code += "  val sum0: Int32 = v0\n";
  for (int i = 1; i < crossing; ++i) {
    code += std::format("  val sum{}: Int32 = sum{} + v{}\n", i, i - 1, i);
  }
  code += std::format("  return sum{}\n}}\n", crossing - 1);
  return code;
}

// `carried` values written on every turn of a loop and read after it, so
// that each has a phi function where the loop is entered, taking one value
// from before it and one from the turn before.
inline std::string LoopCarriedValues(int carried) {
  std::string code = "fun main(): Int32 {\n  mut val i: Int32 = 0\n";
  for (int k = 0; k < carried; ++k) {
    code += std::format("  mut val v{}: Int32 = {}\n", k, k);
  }

  code += "  loop {\n    if i == 3 {\n      break\n    }\n";
  for (int k = 0; k < carried; ++k) {
    code += std::format("    mut v{} = v{} + 1\n", k, k);
  }
  code += "    mut i = i + 1\n  }\n";

  code += "  val s0: Int32 = v0\n";
  for (int k = 1; k < carried; ++k) {
    code += std::format("  val s{}: Int32 = s{} + v{}\n", k, k - 1, k);
  }
  code += std::format("  return s{}\n}}\n", carried - 1);
  return code;
}

}  // namespace lucid
