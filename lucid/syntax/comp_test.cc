#include "lucid/syntax/comp.h"

#include <expected>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

class CompCheckTest : public Test, public AstFixture {
 protected:
  std::expected<void, CompError> CheckComp(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    std::list<FuncDefStmt> func_defs;
    for (Parser parser(syn_ctx_, src, Lexer(code_with_null));;) {
      std::expected<std::optional<Def>, ParserError> def_or_error =
          parser.Parse();
      if (!def_or_error.has_value()) {
        return std::unexpected(CompError("failed to parse definition"));
      }

      std::optional<Def> maybe_def = std::move(def_or_error).value();
      if (!maybe_def.has_value()) break;

      Def def = std::move(maybe_def).value();
      if (const auto* type_def = std::get_if<TypeDefStmt>(&def)) {
        syn_ctx_.RegisterType(type_def->name, type_def->type);
        continue;
      }

      auto& func_def = std::get<FuncDefStmt>(def);
      func_defs.push_back(func_def);
      syn_ctx_.AddFuncDef(func_defs.back());
    }

    FuncDefStmt* test_func_def = nullptr;
    for (auto& func_def : func_defs) {
      if (syn_ctx_.DerefIdent(func_def.name) == "test") {
        test_func_def = &func_def;
      }
    }

    return lucid::CheckComp(syn_ctx_, *test_func_def);
  }
};

TEST(CompCheckTest, EmptyNonCompFunc) {
  std::string_view src = R"(
    fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, EmptyCompFunc) {
  std::string_view src = R"(
    comp fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, DoStmtInCompFunc) {
  std::string_view src = R"(
    fun effect(): Int32 {
      return 0
    }

    comp fun test(): Int32 {
      do effect()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST(CompCheckTest, DoStmtOnCompInCompFunc) {
  std::string_view src = R"(
    comp fun pure(): Int32 {
      return 0
    }

    comp fun test(): Int32 {
      do pure()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST(CompCheckTest, NestedDoStmtInConstFunc) {
  std::string_view src = R"(
    fun effect(): Int32 {
      return 0
    }

    comp fun test(b1: Bool, b2: Bool): Int32 {
      loop {
        if b1 {
        } else {
          if b2 {
            do effect()
          }
        }
      }
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST(CompCheckTest, NonCompVarDeclNonCompInit) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclNonCompInit) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = comp foo()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompFuncCallInit) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = comp foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Int32 = comp 21
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompBoolLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Bool = comp true
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A comp value another comp value is worked out from has to be one that
// stays as it was initialized.
TEST(CompCheckTest, CompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Int32 = comp 21
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A write can reach a variable marked `mut` after it is initialized, so what
// it holds is not known during compilation however it was initialized.
TEST(CompCheckTest, MutCompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    fun test(): Int32 {
      mut val x: Int32 = comp 21
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

// Initializing a variable during compilation says nothing about what may be
// done to it afterwards.
TEST(CompCheckTest, MutCompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      mut val x: Int32 = comp 21
      mut x = x + 1
      return x
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A variable whose initializer is only partly worked out during compilation
// does not hold what compilation worked out, so nothing can be read from it
// there.
TEST(CompCheckTest, PartlyCompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(n: Int32): Int32 {
      val x: Int32 = comp foo() + n
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

// A `comp` reaches one element, so what stands beside it is left for when the
// program runs and may read whatever is in scope.
TEST(CompCheckTest, CompExprBesideARuntimeValue) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(n: Int32): Int32 {
      return comp foo() + n
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A `comp` needs no declaration to stand on any more.
TEST(CompCheckTest, CompExprInReturn) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return comp foo()
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A comp function called without a `comp` on the call is called while the
// program runs, like any other.
TEST(CompCheckTest, CompFuncCalledWithoutComp) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return foo()
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// What a `comp` expression calls has to be a comp function wherever the
// expression stands.
TEST(CompCheckTest, CompExprCallingNonCompFunc) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return comp foo()
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

// A `comp` on a field read reaches what the field is read from as well.
TEST(CompCheckTest, CompFieldReadOfNonCompVariable) {
  std::string_view src = R"(
    val P: Type = (x: Int32)

    fun test(): Int32 {
      mut val p: P
      mut p.x = 1
      return comp p.x
    }
  )";

  EXPECT_EQ(CheckComp(src),
            std::unexpected(
                CompError("non-comp identifier not allowed in comp context")));
}

TEST(CompCheckTest, CompFieldReadOfCallToNonCompFunc) {
  std::string_view src = R"(
    val P: Type = (x: Int32)

    fun foo(): P {
      mut val p: P
      return p
    }

    fun test(): Int32 {
      return comp foo().x
    }
  )";

  EXPECT_EQ(CheckComp(src),
            std::unexpected(CompError(
                "calling non-comp function not allowed in comp context")));
}

// An array is left in the caller's frame however the call is made, so a call
// returning one needs no `comp`.
TEST(CompCheckTest, ArrayResultCalledWhileTheProgramRuns) {
  std::string_view src = R"(
    fun foo(n: Int32): Int32[2] {
      mut val a: Int32[2]
      mut a[0] = n
      return a
    }

    fun test(n: Int32): Int32 {
      val t: Int32[2] = foo(n)
      return t[0] + foo(n)[1]
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, ArrayResultInitializingAVariable) {
  std::string_view src = R"(
    comp fun foo(): Int32[2] {
      mut val a: Int32[2]
      return a
    }

    fun test(): Int32 {
      val t: Int32[2] = comp foo()
      return t[0] + comp foo()[1]
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// The array a call leaves in the frame is taken from there only by what it
// initialises, by a return, by an index and by a field read.
TEST(CompCheckTest, ArrayResultAsAnArgument) {
  std::string_view src = R"(
    comp fun foo(): Int32[2] {
      mut val a: Int32[2]
      return a
    }

    comp fun bar(n: Int32): Int32 {
      return n
    }

    fun test(): Int32 {
      return comp bar(foo())
    }
  )";

  EXPECT_EQ(CheckComp(src),
            std::unexpected(CompError(
                "an array or a tuple a call returns can only be assigned, be "
                "returned, be indexed or have a field read")));
}

TEST(CompCheckTest, TupleResultAsAnArgument) {
  std::string_view src = R"(
    val P: Type = (x: Int32)

    fun foo(): P {
      mut val p: P
      return p
    }

    fun bar(n: Int32): Int32 {
      return n
    }

    fun test(): Int32 {
      return bar(foo())
    }
  )";

  EXPECT_EQ(CheckComp(src),
            std::unexpected(CompError(
                "an array or a tuple a call returns can only be assigned, be "
                "returned, be indexed or have a field read")));
}

TEST(CompCheckTest, TupleResultFieldRead) {
  std::string_view src = R"(
    val P: Type = (x: Int32, v: Int32[2])

    fun foo(): P {
      mut val p: P
      return p
    }

    fun test(): Int32 {
      val p: P = foo()
      return p.x + foo().x + foo().v[1]
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// What is returned is copied out of wherever it lies, which can be inside
// another value.
TEST(CompCheckTest, ArrayReturnedFromAField) {
  std::string_view src = R"(
    val Row: Type = (v: Int32[2])

    comp fun test(): Int32[2] {
      mut val r: Row
      return r.v
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A call's result is copied into what it is assigned to like any other value.
TEST(CompCheckTest, FrameResultAssigned) {
  std::string_view src = R"(
    val P: Type = (x: Int32)
    val Q: Type = (p: P)

    fun foo(): P {
      mut val p: P
      return p
    }

    fun test(): Int32 {
      mut val p: P
      mut p = foo()
      mut val a: P[2]
      mut a[1] = foo()
      mut val q: Q
      mut q.p = foo()
      return p.x
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

}  // namespace
}  // namespace lucid
