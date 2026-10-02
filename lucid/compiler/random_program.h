#pragma once

#include <array>
#include <cstdint>
#include <format>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

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
// different roads. It is worked out in the width the language holds a number
// in, so that a sum running past what that width holds is expected to come
// back round rather than to be wrong.
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
    numbers_ = HashMap<std::string, std::int32_t>();
    flags_ = HashMap<std::string, bool>();
    known_ = HashSet<std::string>();
    next_name_ = 0;

    Preamble();

    // Sometimes more parameters than there are registers to pass them in, so
    // that the ones left on the stack are passed and taken.
    const int params = Between(0, 14);
    const std::vector<std::int32_t> args = GenerateCallee(params);
    Line(0, "");
    const std::int32_t result = GenerateMain(params, args);

    std::string source;
    for (const std::string& line : lines_) source += line + "\n";
    return {.source = std::move(source), .exit_code = result};
  }

 private:
  // What a number expression is, and what it comes to.
  struct Value {
    std::string text;
    std::int32_t number;

    // Whether compilation could work it out: everything it is made of is a
    // literal, a name that holds what compilation worked out, or a call to a
    // comp function over those.
    bool known = false;
  };

  // What a condition is, and whether it holds.
  struct Cond {
    std::string text;
    bool holds;
  };

  // The names a piece of the program can read, by what they hold, and the
  // ones a write can reach.
  //
  // A declaration says with a `mut` whether it may be written, so what is
  // written has to be chosen from among the ones that said so. Not every
  // declaration says it, which is what keeps the other kind in the programs
  // as well.
  struct Scope {
    std::vector<std::string> numbers;
    std::vector<std::string> writable;
    std::vector<std::string> flags;

    // The numbers that hold what compilation worked out, which a `comp` can
    // read. They are a few among the rest, so they are picked from apart
    // from them some of the time, or a `comp` would rarely come to read one.
    std::vector<std::string> known;
  };

  int Between(int low, int high) {
    return std::uniform_int_distribution<int>(low, high)(rng_);
  }

  bool Chance(double odds) {
    return std::uniform_real_distribution<double>(0, 1)(rng_) < odds;
  }

  template <typename T>
  const T& Any(const std::vector<T>& of) {
    return of[Between(0, of.size() - 1)];
  }

  void Line(int depth, std::string text) {
    lines_.push_back(std::string(depth * 2, ' ') + std::move(text));
  }

  std::string Name() { return std::format("v{}", next_name_++); }

  // Arithmetic in the width the language holds a number in. Carried out over
  // the width above so that running past the end of it is coming back round
  // rather than something the language has no answer for.
  static std::int32_t Narrow(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
  }

  // Functions of a known shape, so that a call can stand anywhere an
  // expression can and still be worked out here.
  void Preamble() {
    Line(0, "val Point: Type = (x: Int32, y: Int32)");
    Line(0, "");
    Line(0, "fun printString(s: String): Int32 {");
    Line(1, "return 0");
    Line(0, "}");
    Line(0, "");
    Line(0, "comp fun twice(a: Int32): Int32 {");
    Line(1, "return a + a");
    Line(0, "}");
    Line(0, "");
    Line(0, "fun chosen(a: String, b: String, take: Int32): String {");
    Line(1, "if take > 0 {");
    Line(2, "return a");
    Line(1, "}");
    Line(1, "return b");
    Line(0, "}");
    Line(0, "");
    Line(0, "comp fun larger(a: Int32, b: Int32): Int32 {");
    Line(1, "if a > b {");
    Line(2, "return a");
    Line(1, "}");
    Line(1, "return b");
    Line(0, "}");
    Line(0, "");
  }

  // A number to write as it is. Mostly a small one, and sometimes one near the
  // edge of the width, so that a sum or a product of a few of them runs past
  // it and comes back round. That is where working a number out during
  // compilation has disagreed with working it out running, and a number that
  // wide is also one no instruction carries in a field of its own.
  std::int32_t Literal() {
    static constexpr std::array<std::int32_t, 5> kWide = {
        2147483647, 2147483646, 1073741824, 65536, 46341};
    if (Chance(0.15)) return kWide[Between(0, kWide.size() - 1)];
    return Between(0, 9);
  }

  // A number, built out of what is live and what can be done to it.
  //
  // One that compilation could work out is sometimes asked for there, with a
  // `comp`. What it comes to is the same either way, so the program's answer
  // checks the work done while compiling against the work done running, the
  // edges of the width included.
  Value Number(const Scope& scope, int depth) {
    Value value = NumberOf(scope, depth);
    if (value.known && Chance(0.15)) {
      value.text = std::format("comp ({})", value.text);
    }
    return value;
  }

  // Everything binds explicitly, because what this has to agree with is the
  // compiler's arithmetic rather than its reading of an expression, and the
  // tests written by hand already say how an expression is read.
  Value NumberOf(const Scope& scope, int depth) {
    if (depth <= 0 || scope.numbers.empty() || Chance(0.25)) {
      const std::int32_t value = Literal();
      return {.text = std::format("{}", value), .number = value, .known = true};
    }

    switch (Between(0, 6)) {
      case 0: {
        const std::string& name = !scope.known.empty() && Chance(0.3)
                                      ? Any(scope.known)
                                      : Any(scope.numbers);
        return {.text = name,
                .number = *numbers_.Get(name),
                .known = known_.Contains(name)};
      }
      case 1:
      case 2: {
        // Add, subtract or multiply, any of which can run past the width.
        const Value lhs = Number(scope, depth - 1);
        const Value rhs = Number(scope, depth - 1);
        switch (Between(0, 2)) {
          case 0:
            return {.text = std::format("({} + {})", lhs.text, rhs.text),
                    .number = Narrow(std::int64_t{lhs.number} + rhs.number),
                    .known = lhs.known && rhs.known};
          case 1:
            return {.text = std::format("({} - {})", lhs.text, rhs.text),
                    .number = Narrow(std::int64_t{lhs.number} - rhs.number),
                    .known = lhs.known && rhs.known};
          default:
            return {.text = std::format("({} * {})", lhs.text, rhs.text),
                    .number = Narrow(std::int64_t{lhs.number} * rhs.number),
                    .known = lhs.known && rhs.known};
        }
      }
      case 3: {
        // Divided or divided into, by a number that cannot be zero and
        // cannot be negative, so that neither dividing by nothing nor the
        // one division that has no answer can come up.
        const Value lhs = Number(scope, depth - 1);
        const std::int32_t by = Between(1, 9);
        const bool remainder = Chance(0.5);
        return {.text = std::format("({} {} {})", lhs.text,
                                    remainder ? '%' : '/', by),
                .number = remainder ? lhs.number % by : lhs.number / by,
                .known = lhs.known};
      }
      case 4: {
        const Value a = Number(scope, depth - 1);
        return {.text = std::format("twice({})", a.text),
                .number = Narrow(std::int64_t{a.number} + a.number),
                .known = a.known};
      }
      case 5: {
        const Value a = Number(scope, depth - 1);
        const Value b = Number(scope, depth - 1);
        return {.text = std::format("larger({}, {})", a.text, b.text),
                .number = a.number > b.number ? a.number : b.number,
                .known = a.known && b.known};
      }
      default: {
        // A condition, counted as one or nothing, which is what keeps a
        // `Bool` alive far enough to want a register.
        const Cond cond = Boolean(scope, depth - 1);
        const std::string name = Name();
        Line(pending_depth_, std::format("mut val {}: Int32 = 0", name));
        Line(pending_depth_, std::format("if {} {{", cond.text));
        Line(pending_depth_ + 1, std::format("mut {} = 1", name));
        Line(pending_depth_, "}");
        numbers_.Set(name, cond.holds ? 1 : 0);
        return {.text = name, .number = cond.holds ? 1 : 0};
      }
    }
  }

  // A condition, out of comparisons joined and negated.
  Cond Boolean(const Scope& scope, int depth) {
    if (depth > 0 && !scope.flags.empty() && Chance(0.2)) {
      const std::string& name = Any(scope.flags);
      return {.text = name, .holds = *flags_.Get(name)};
    }
    if (depth > 0 && Chance(0.3)) {
      const Cond lhs = Boolean(scope, depth - 1);
      const Cond rhs = Boolean(scope, depth - 1);
      const bool both = Chance(0.5);
      return {.text = std::format("({} {} {})", lhs.text, both ? "and" : "or",
                                  rhs.text),
              .holds = both ? lhs.holds && rhs.holds : lhs.holds || rhs.holds};
    }
    if (depth > 0 && Chance(0.2)) {
      const Cond inner = Boolean(scope, depth - 1);
      return {.text = std::format("!({})", inner.text), .holds = !inner.holds};
    }

    const Value lhs = Number(scope, depth - 1);
    const Value rhs = Number(scope, depth - 1);
    switch (Between(0, 5)) {
      case 0:
        return {std::format("({} > {})", lhs.text, rhs.text),
                lhs.number > rhs.number};
      case 1:
        return {std::format("({} < {})", lhs.text, rhs.text),
                lhs.number < rhs.number};
      case 2:
        return {std::format("({} >= {})", lhs.text, rhs.text),
                lhs.number >= rhs.number};
      case 3:
        return {std::format("({} <= {})", lhs.text, rhs.text),
                lhs.number <= rhs.number};
      case 4:
        return {std::format("({} == {})", lhs.text, rhs.text),
                lhs.number == rhs.number};
      default:
        return {std::format("({} != {})", lhs.text, rhs.text),
                lhs.number != rhs.number};
    }
  }

  // A declaration of a number, sometimes one written as a whole with `comp`.
  // One of those that no write can reach holds what compilation worked out,
  // and becomes a name that a later `comp` can read. One a write can reach is
  // only started from that, and is a name like any other.
  void DeclareNumber(int depth, Scope& scope) {
    pending_depth_ = depth;
    Value value = NumberOf(scope, 2);
    const std::string name = Name();
    const bool writable = Chance(0.6);
    const bool comp = value.known && Chance(0.5);
    if (comp) value.text = std::format("comp ({})", value.text);
    Line(depth, std::format("{}val {}: Int32 = {}", writable ? "mut " : "",
                            name, value.text));
    numbers_.Set(name, value.number);
    scope.numbers.push_back(name);
    if (writable) scope.writable.push_back(name);
    if (comp && !writable) {
      known_.Insert(name);
      scope.known.push_back(name);
    }
  }

  void DeclareFlag(int depth, Scope& scope) {
    pending_depth_ = depth;
    const Cond cond = Boolean(scope, 2);
    const std::string name = Name();
    Line(depth, std::format("val {}: Bool = {}", name, cond.text));
    flags_.Set(name, cond.holds);
    scope.flags.push_back(name);
  }

  // A run of statements, with branches and loops among them.
  //
  // What is declared before the run stays live to the end of the function,
  // so every one of those crosses every merge the run makes. That is what
  // asks the allocator for more registers than there are to give.
  void Body(int depth, Scope& scope, int budget) {
    for (int i = 0; i < budget; ++i) {
      if (depth >= 2 || Chance(0.35)) {
        DeclareNumber(depth, scope);
      } else if (Chance(0.15)) {
        DeclareFlag(depth, scope);
      } else if (!scope.writable.empty() && Chance(0.3)) {
        pending_depth_ = depth;
        const std::string target = Any(scope.writable);
        const Value value = Number(scope, 2);
        Line(depth, std::format("mut {} = {}", target, value.text));
        numbers_.Set(target, value.number);
      } else if (!scope.numbers.empty() && Chance(0.6)) {
        Branch(depth, scope);
      } else if (!scope.numbers.empty()) {
        Loop(depth, scope);
      } else {
        DeclareNumber(depth, scope);
      }
    }
  }

  // A branch whose sides write to what crosses it, so that what crosses has
  // a phi function where the sides meet. Both sides are written out; only
  // what the taken one does is counted.
  void Branch(int depth, Scope& scope) {
    pending_depth_ = depth;
    const Cond cond = Boolean(scope, 2);

    std::vector<std::pair<std::string, std::int32_t>> then_writes;
    std::vector<std::pair<std::string, std::int32_t>> else_writes;
    Line(depth, std::format("if {} {{", cond.text));
    for (const std::string& name : scope.writable) {
      if (!Chance(0.5)) continue;

      Line(depth + 1, std::format("mut {} = {} + 1", name, name));
      then_writes.emplace_back(name,
                               Narrow(std::int64_t{*numbers_.Get(name)} + 1));
    }
    Line(depth, "} else {");
    for (const std::string& name : scope.writable) {
      if (!Chance(0.5)) continue;

      Line(depth + 1, std::format("mut {} = {} - 1", name, name));
      else_writes.emplace_back(name,
                               Narrow(std::int64_t{*numbers_.Get(name)} - 1));
    }
    Line(depth, "}");

    for (const auto& [name, value] : cond.holds ? then_writes : else_writes) {
      numbers_.Set(name, value);
    }
  }

  // A loop that runs a fixed number of turns, carrying what is live around
  // it, so that each of those has a phi function where the loop is entered.
  void Loop(int depth, Scope& scope) {
    const std::string counter = Name();
    const int turns = Between(1, 3);
    Line(depth, std::format("mut val {}: Int32 = 0", counter));
    numbers_.Set(counter, turns);

    Line(depth, "loop {");
    Line(depth + 1, std::format("if {} >= {} {{", counter, turns));
    Line(depth + 2, "break");
    Line(depth + 1, "}");
    for (const std::string& name : scope.writable) {
      if (!Chance(0.5)) continue;

      const int step = Between(1, 3);
      Line(depth + 1, std::format("mut {} = {} + {}", name, name, step));
      numbers_.Set(name, Narrow(std::int64_t{*numbers_.Get(name)} +
                                std::int64_t{step} * turns));
    }
    Line(depth + 1, std::format("mut {} = {} + 1", counter, counter));
    Line(depth, "}");
    scope.numbers.push_back(counter);
    scope.writable.push_back(counter);
  }

  // Numbers worked out while compiling, each from the ones before it, which
  // is what a `comp` reading a name relies on: a name holds what compilation
  // worked out where the whole of what initialises it does and no write can
  // reach it. A number built here that compilation could not work out is
  // declared as any other would be.
  void Known(int depth, Scope& scope) {
    pending_depth_ = depth;
    for (int i = Between(1, 3); i > 0; --i) {
      Scope from_known;
      from_known.numbers = scope.known;
      Value value = Chance(0.3) ? Wrapped() : NumberOf(from_known, 2);

      const std::string name = Name();
      if (value.known) {
        Line(depth, std::format("val {}: Int32 = comp ({})", name, value.text));
        known_.Insert(name);
        scope.known.push_back(name);
      } else {
        Line(depth, std::format("val {}: Int32 = {}", name, value.text));
      }
      numbers_.Set(name, value.number);
      scope.numbers.push_back(name);
    }
  }

  // A number that runs past the width while being worked out, and is then
  // compared or divided, both of which look at what it came back round to.
  // Put into the program as it is, a number held wider than the width would
  // be cut back to the right one on its way into a register; only looking at
  // it during compilation shows whether it came back round there too.
  Value Wrapped() {
    static constexpr std::array<std::int32_t, 3> kWide = {2147483647,
                                                          1073741824, 65536};
    const std::int32_t a = kWide[Between(0, kWide.size() - 1)];
    const std::int32_t b = kWide[Between(0, kWide.size() - 1)];
    const bool product = Chance(0.5);
    const std::int32_t wrapped =
        Narrow(product ? std::int64_t{a} * b : std::int64_t{a} + b);
    const std::string text =
        std::format("({} {} {})", a, product ? '*' : '+', b);

    const std::int32_t other = Between(0, 9);
    if (Chance(0.5)) {
      return {.text = std::format("larger({}, {})", text, other),
              .number = wrapped > other ? wrapped : other,
              .known = true};
    }
    const std::int32_t by = Between(1, 9);
    return {.text = std::format("({} / {})", text, by),
            .number = wrapped / by,
            .known = true};
  }

  // An array, read through an index, which is what reaches a slot of the
  // frame the furthest from where the frame begins.
  void Array(int depth, Scope& scope) {
    const std::string buffer = Name();
    const int size = Between(1, 40);
    Line(depth, std::format("mut val {}: Int32[{}]", buffer, size));

    std::vector<std::int32_t> held(size);
    for (int i = 0; i < size; ++i) {
      held[i] = Between(0, 9);
      Line(depth, std::format("mut {}[{}] = {}", buffer, i, held[i]));
    }

    const int at = Between(0, size - 1);
    const std::string name = Name();
    Line(depth, std::format("val {}: Int32 = {}[{}]", name, buffer, at));
    numbers_.Set(name, held[at]);
    scope.numbers.push_back(name);
  }

  // Tuples, and an array of them, which lay a frame out in slots of their
  // own rather than in a run of one width.
  void Tuples(int depth, Scope& scope) {
    const std::string points = Name();
    const int size = Between(1, 6);
    Line(depth, std::format("mut val {}: Point[{}]", points, size));

    std::vector<std::pair<std::int32_t, std::int32_t>> held(size);
    for (int i = 0; i < size; ++i) {
      held[i] = {Between(0, 9), Between(0, 9)};
      Line(depth, std::format("mut {}[{}].x = {}", points, i, held[i].first));
      Line(depth, std::format("mut {}[{}].y = {}", points, i, held[i].second));
    }

    const int at = Between(0, size - 1);
    const std::string name = Name();
    Line(depth, std::format("val {}: Int32 = {}[{}].x + {}[{}].y", name, points,
                            at, points, at));
    numbers_.Set(name, Narrow(std::int64_t{held[at].first} + held[at].second));
    scope.numbers.push_back(name);
  }

  // Returns what the call should be passed.
  std::vector<std::int32_t> GenerateCallee(int params) {
    std::string signature = "fun callee(";
    Scope scope;
    std::vector<std::int32_t> args;
    for (int i = 0; i < params; ++i) {
      if (i > 0) signature += ", ";

      // Some of them the body may write, which is what the `mut` on a
      // parameter says, and some it may only read.
      const bool writable = Chance(0.5);
      signature += std::format("{}p{}: Int32", writable ? "mut " : "", i);

      args.push_back(Between(0, 9));
      numbers_.Set(std::format("p{}", i), args.back());
      scope.numbers.push_back(std::format("p{}", i));
      if (writable) scope.writable.push_back(std::format("p{}", i));
    }
    Line(0, signature + "): Int32 {");

    if (Chance(0.4)) Array(1, scope);
    if (Chance(0.5)) Known(1, scope);
    if (Chance(0.3)) Tuples(1, scope);
    Body(1, scope, Between(1, 5));
    Line(1, std::format("return {}", Sum(scope)));
    Line(0, "}");

    callee_result_ = SumOf(scope);
    return args;
  }

  std::int32_t GenerateMain(int params, const std::vector<std::int32_t>& args) {
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

    Scope scope;
    if (Chance(0.4)) Array(1, scope);
    if (Chance(0.5)) Known(1, scope);
    if (Chance(0.3)) Tuples(1, scope);
    Body(1, scope, Between(1, 5));

    std::string call = "callee(";
    for (std::size_t i = 0; i < args.size(); ++i) {
      if (i > 0) call += ", ";
      call += std::format("{}", args[i]);
    }
    const std::string result = Name();
    Line(1, std::format("val {}: Int32 = {})", result, call));
    numbers_.Set(result, callee_result_);
    scope.numbers.push_back(result);

    // Some of them by way of a function that hands a string back, which is
    // a result twice the width a number is.
    for (const std::string& name : strings) {
      if (strings.size() > 1 && Chance(0.5)) {
        Line(1, std::format("do printString(chosen({}, {}, {}))", name,
                            Any(strings), Between(0, 1)));
      } else {
        Line(1, std::format("do printString({})", name));
      }
    }
    Body(1, scope, Between(0, 3));

    // What every condition still live came to, counted in, so that a `Bool`
    // is worth holding on to as far as here.
    const std::string total = Name();
    Line(1, std::format("mut val {}: Int32 = {}", total, Sum(scope)));
    std::int64_t sum = SumOf(scope);
    for (const std::string& flag : scope.flags) {
      Line(1, std::format("if {} {{", flag));
      Line(2, std::format("mut {} = {} + 1", total, total));
      Line(1, "}");
      if (*flags_.Get(flag)) ++sum;
    }

    // Folded into what an exit code holds, and never negative, so that what
    // the program returns is what it is compared against.
    const std::int32_t whole = Narrow(sum);
    Line(1, std::format("val folded: Int32 = {} - {} / 251 * 251", total, total,
                        total));
    Line(1, "if folded >= 0 {");
    Line(2, "return folded");
    Line(1, "}");
    Line(1, "return folded + 251");
    Line(0, "}");

    const std::int32_t folded = whole - whole / 251 * 251;
    return folded >= 0 ? folded : folded + 251;
  }

  static std::string Sum(const Scope& scope) {
    if (scope.numbers.empty()) return "0";

    std::string sum = scope.numbers[0];
    for (std::size_t i = 1; i < scope.numbers.size(); ++i) {
      sum += " + " + scope.numbers[i];
    }
    return sum;
  }

  std::int64_t SumOf(const Scope& scope) {
    std::int64_t total = 0;
    for (const std::string& name : scope.numbers) {
      total = Narrow(total + *numbers_.Get(name));
    }
    return total;
  }

  std::mt19937_64 rng_;
  std::vector<std::string> lines_;
  HashMap<std::string, std::int32_t> numbers_;
  HashMap<std::string, bool> flags_;

  // The names that hold what compilation worked out.
  HashSet<std::string> known_;
  std::int32_t callee_result_ = 0;
  int next_name_ = 0;

  // Where a statement an expression needs goes, which is beside the
  // statement the expression is part of.
  int pending_depth_ = 0;
};

}  // namespace lucid
