#include "lucid/syntax/comp.h"

#include <utility>

#include "gtest/gtest.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

class CompCheckTest : public testing::Test, public AstFixture {
 protected:
  std::expected<void, CompError> CheckComp(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    std::vector<FuncDefStmt> func_defs;
    for (Parser parser(ctx_, src, Lexer(code_with_null));;) {
      auto maybe_func_def_stmt = parser.ParseFuncDef();
      auto ref = std::move(maybe_func_def_stmt).value();
      if (!ref.has_value()) break;

      func_defs.push_back(std::move(*ref));
    }

    const FuncDefStmt* test_func_def = nullptr;
    for (const auto& func_def : func_defs) {
      if (ctx_.DerefIdent(func_def.name) == "test") test_func_def = &func_def;
    }

    return lucid::CheckComp(ctx_, func_defs, *test_func_def);
  }
};

TEST_F(CompCheckTest, EmptyNonCompFunc) {
  std::string_view src = R"(
    let test = () -> Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, EmptyCompFunc) {
  std::string_view src = R"(
    comp let test = () -> Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, DoStmtInCompFunc) {
  std::string_view src = R"(
    let effect = () -> Int32 {
      return 0
    }

    comp let test = () -> Int32 {
      do effect()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, DoStmtOnCompInCompFunc) {
  std::string_view src = R"(
    comp let pure = () -> Int32 {
      return 0
    }

    comp let test = () -> Int32 {
      do pure()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, NestedDoStmtInConstFunc) {
  std::string_view src = R"(
    let effect = () -> Int32 {
      return 0
    }

    comp let test = (b1: Bool, b2: Bool) -> Int32 {
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

TEST_F(CompCheckTest, NonCompVarDeclNonCompInit) {
  std::string_view src = R"(
    let foo = () -> Int32 {
      return 0
    }

    let test = () -> Int32 {
      let x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclNonCompInit) {
  std::string_view src = R"(
    let foo = () -> Int32 {
      return 0
    }

    let test = () -> Int32 {
      comp let x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompFuncCallInit) {
  std::string_view src = R"(
    comp let foo = () -> Int32 {
      return 0
    }

    let test = () -> Int32 {
      comp let x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    let test = () -> Int32 {
      comp let x: Int32 = 21
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompBoolLitInit) {
  std::string_view src = R"(
    let test = () -> Int32 {
      comp let x: Bool = true
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

}  // namespace
}  // namespace lucid
