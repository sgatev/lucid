#pragma once

#include <cstdint>
#include <format>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "lucid/core/container/hash_map.h"

namespace lucid {

// Builds a random program, and works out what it comes to while building it.
//
// The shapes it is made of are the ones that have hidden mistakes in the
// compiler before now, and what each is for is written beside it below. They
// have in common that they crowd some part of the machine a program has to
// be fitted into: the registers a value can be held in, the slots a frame is
// made of, the registers a call passes its arguments in. A program that
// crowds none of them exercises little, because the easy paths are the ones
// the tests written by hand already cover.
//
// What the program returns is worked out here rather than read back from
// anything the compiler builds, so that the two answers are arrived at by
// different roads.
class RandomProgram {
 public:
  explicit RandomProgram(std::uint64_t seed) : rng_(seed) {}

  // The source of a program, and the exit code running it should give.
  struct Program {
    std::string source;
    int exit_code;
  };

  Program Generate() {
    lines_.clear();
    values_ = HashMap<std::string, std::int64_t>();
    next_name_ = 0;

    // Sometimes more parameters than there are registers to pass them in, so
    // that the ones left on the stack are passed and taken.
    const int params = Between(0, 14);

    Line(0, "fun printString(s: String): Int32 {");
    Line(1, "return 0");
    Line(0, "}");
    Line(0, "");
    const std::vector<int> args = GenerateCallee(params);
    Line(0, "");
    const std::int64_t result = GenerateMain(params, args);

    std::string source;
    for (const std::string& line : lines_) source += line + "\n";
    return {.source = std::move(source), .exit_code = static_cast<int>(result)};
  }

 private:
  int Between(int low, int high) {
    return std::uniform_int_distribution<int>(low, high)(rng_);
  }

  bool Chance(double odds) {
    return std::uniform_real_distribution<double>(0, 1)(rng_) < odds;
  }

  void Line(int depth, std::string text) {
    lines_.push_back(std::string(depth * 2, ' ') + std::move(text));
  }

  std::string Name() { return std::format("v{}", next_name_++); }

  std::int64_t ValueOf(const std::string& name) { return *values_.Get(name); }

  // An expression over what is live, beside what it comes to.
  std::pair<std::string, std::int64_t> Expr(
      const std::vector<std::string>& live) {
    if (live.empty() || Chance(0.25)) {
      const int value = Between(0, 9);
      return {std::format("{}", value), value};
    }

    const std::string& a = live[Between(0, live.size() - 1)];
    if (Chance(0.4)) return {a, ValueOf(a)};

    const std::string& b = live[Between(0, live.size() - 1)];
    const bool adds = Chance(0.5);
    return {std::format("{} {} {}", a, adds ? '+' : '-', b),
            adds ? ValueOf(a) + ValueOf(b) : ValueOf(a) - ValueOf(b)};
  }

  void Declare(int depth, std::vector<std::string>& live) {
    const auto [text, value] = Expr(live);
    const std::string name = Name();
    Line(depth, std::format("val {}: Int32 = {}", name, text));
    values_.Set(name, value);
    live.push_back(name);
  }

  // A run of statements, with branches and loops among them.
  //
  // What is declared before the run stays live to the end of the function,
  // so every one of those crosses every merge the run makes. That is what
  // asks the allocator for more registers than there are to give.
  void Body(int depth, std::vector<std::string>& live, int budget) {
    for (int i = 0; i < budget; ++i) {
      if (depth >= 2 || Chance(0.4)) {
        Declare(depth, live);
      } else if (!live.empty() && Chance(0.3)) {
        const std::string target = live[Between(0, live.size() - 1)];
        const auto [text, value] = Expr(live);
        Line(depth, std::format("&{} = {}", target, text));
        values_.Set(target, value);
      } else if (!live.empty() && Chance(0.6)) {
        Branch(depth, live);
      } else if (!live.empty()) {
        Loop(depth, live);
      } else {
        Declare(depth, live);
      }
    }
  }

  // A branch whose sides write to what crosses it, so that what crosses has
  // a phi function where the sides meet. Both sides are written out; only
  // what the taken one does is counted.
  void Branch(int depth, std::vector<std::string>& live) {
    const std::string a = live[Between(0, live.size() - 1)];
    const std::string b = live[Between(0, live.size() - 1)];
    const bool taken = ValueOf(a) > ValueOf(b);

    std::vector<std::pair<std::string, std::int64_t>> then_writes;
    std::vector<std::pair<std::string, std::int64_t>> else_writes;
    Line(depth, std::format("if {} > {} {{", a, b));
    for (const std::string& name : live) {
      if (!Chance(0.5)) continue;

      Line(depth + 1, std::format("&{} = {} + 1", name, name));
      then_writes.emplace_back(name, ValueOf(name) + 1);
    }
    Line(depth, "} else {");
    for (const std::string& name : live) {
      if (!Chance(0.5)) continue;

      Line(depth + 1, std::format("&{} = {} - 1", name, name));
      else_writes.emplace_back(name, ValueOf(name) - 1);
    }
    Line(depth, "}");

    for (const auto& [name, value] : taken ? then_writes : else_writes) {
      values_.Set(name, value);
    }
  }

  // A loop that runs a fixed number of turns, carrying what is live around
  // it, so that each of those has a phi function where the loop is entered.
  void Loop(int depth, std::vector<std::string>& live) {
    const std::string counter = Name();
    const int turns = Between(1, 3);
    Line(depth, std::format("val {}: Int32 = 0", counter));
    values_.Set(counter, turns);

    Line(depth, "loop {");
    Line(depth + 1, std::format("if {} >= {} {{", counter, turns));
    Line(depth + 2, "break");
    Line(depth + 1, "}");
    for (const std::string& name : live) {
      if (!Chance(0.5)) continue;

      const int step = Between(1, 3);
      Line(depth + 1, std::format("&{} = {} + {}", name, name, step));
      values_.Set(name, ValueOf(name) + std::int64_t{step} * turns);
    }
    Line(depth + 1, std::format("&{} = {} + 1", counter, counter));
    Line(depth, "}");
    live.push_back(counter);
  }

  // An array, read through an index, which is what reaches a slot of the
  // frame the furthest from where the frame begins. Long enough, sometimes,
  // that the slots of it stand past what one instruction can reach.
  void Array(int depth, std::vector<std::string>& live) {
    const std::string buffer = Name();
    const int size = Between(1, 40);
    Line(depth, std::format("val {}: Int32[{}]", buffer, size));

    std::vector<int> held(size);
    for (int i = 0; i < size; ++i) {
      held[i] = Between(0, 9);
      Line(depth, std::format("&{}[{}] = {}", buffer, i, held[i]));
    }

    const int at = Between(0, size - 1);
    const std::string name = Name();
    Line(depth, std::format("val {}: Int32 = {}[{}]", name, buffer, at));
    values_.Set(name, held[at]);
    live.push_back(name);
  }

  // Returns what the call should be passed.
  std::vector<int> GenerateCallee(int params) {
    std::string signature = "fun callee(";
    std::vector<std::string> live;
    std::vector<int> args;
    for (int i = 0; i < params; ++i) {
      if (i > 0) signature += ", ";
      signature += std::format("p{}: Int32", i);

      args.push_back(Between(0, 9));
      values_.Set(std::format("p{}", i), args.back());
      live.push_back(std::format("p{}", i));
    }
    Line(0, signature + "): Int32 {");

    if (Chance(0.5)) Array(1, live);
    Body(1, live, Between(1, 5));
    Line(1, std::format("return {}", Sum(live)));
    Line(0, "}");

    callee_result_ = SumOf(live);
    return args;
  }

  std::int64_t GenerateMain(int params, const std::vector<int>& args) {
    Line(0, "fun main(): Int32 {");

    // Strings held across the call, because they are the only values in the
    // language wider than the half of a register a number takes, and so the
    // only ones that show a register handed back with its upper half lost.
    std::vector<std::string> strings;
    for (int i = 0, count = Between(0, 9); i < count; ++i) {
      strings.push_back(Name());
      Line(1, std::format("val {}: String = \"{}\"", strings.back(),
                          std::string(i, '.')));
    }

    std::vector<std::string> live;
    if (Chance(0.5)) Array(1, live);
    Body(1, live, Between(1, 5));

    std::string call = "callee(";
    for (std::size_t i = 0; i < args.size(); ++i) {
      if (i > 0) call += ", ";
      call += std::format("{}", args[i]);
    }
    const std::string result = Name();
    Line(1, std::format("val {}: Int32 = {})", result, call));
    values_.Set(result, callee_result_);
    live.push_back(result);

    for (const std::string& name : strings) {
      Line(1, std::format("do printString({})", name));
    }
    Body(1, live, Between(0, 3));

    // Folded into what an exit code holds, and never negative, so that what
    // the program returns is what it is compared against.
    const std::int64_t total = SumOf(live);
    Line(1, std::format("val total: Int32 = {}", Sum(live)));
    Line(1, "val folded: Int32 = total - total / 251 * 251");
    Line(1, "if folded >= 0 {");
    Line(2, "return folded");
    Line(1, "}");
    Line(1, "return folded + 251");
    Line(0, "}");

    const std::int64_t folded = total - total / 251 * 251;
    return folded >= 0 ? folded : folded + 251;
  }

  static std::string Sum(const std::vector<std::string>& live) {
    if (live.empty()) return "0";

    std::string sum = live[0];
    for (std::size_t i = 1; i < live.size(); ++i) sum += " + " + live[i];
    return sum;
  }

  std::int64_t SumOf(const std::vector<std::string>& live) {
    std::int64_t total = 0;
    for (const std::string& name : live) total += ValueOf(name);
    return total;
  }

  std::mt19937_64 rng_;
  std::vector<std::string> lines_;
  HashMap<std::string, std::int64_t> values_;
  std::int64_t callee_result_ = 0;
  int next_name_ = 0;
};

}  // namespace lucid
