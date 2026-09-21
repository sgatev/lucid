#include "lucid/syntax/liveness.h"

#include <cstddef>
#include <string_view>

#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

class SyntaxLivenessAnalysisTest : public Test, public AstFixture {
 protected:
  using State = SyntaxLivenessAnalysis::State;

  std::vector<std::optional<State>> AnalyzeLiveness(FuncDefStmt func_def) {
    auto syn_cfg = ::lucid::BuildControlFlowGraph(syn_ctx_, func_def);
    SyntaxLivenessAnalysis analysis(syn_ctx_, syn_cfg);
    return RunDataflow(Backward(syn_cfg), analysis);
  }

  StmtRef DeclareInt32Var(std::string_view name, int value) {
    return S(VarDeclStmt{
        .name = I(name),
        .type_constraint = T("Int32"),
        .init = E(IntLitExpr{.value = value}),
    });
  }
};

TEST(SyntaxLivenessAnalysisTest, EmptyFunc) {
  auto func_def = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
  };

  EXPECT_THAT(AnalyzeLiveness(func_def),
              Elements(Optional(Field(&State::live_in, IsEmpty())),
                       Optional(Field(&State::live_in, IsEmpty()))));
}

TEST(SyntaxLivenessAnalysisTest, VariableReadInTheBlockThatWroteIt) {
  const auto states = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          DeclareInt32Var("x", 21),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ASSERT_THAT(states, SizeIs(2));

  EXPECT_THAT(states[0]->live_in, IsEmpty());
  EXPECT_THAT(states[1]->live_in, IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, VariableNeverRead) {
  const auto states = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          DeclareInt32Var("x", 21),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = true}),
              .then_stmts = StmtListOf({}),
              .else_stmts = StmtListOf({}),
          }),
          S(ReturnStmt{.value = E(IntLitExpr{.value = 0})}),
      }),
  });

  ASSERT_THAT(states, SizeIs(4));

  for (const auto& state : states) {
    EXPECT_THAT(state->live_in, IsEmpty());
  }
}

TEST(SyntaxLivenessAnalysisTest, VariableReadOnOneBranchOnly) {
  const auto states = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          DeclareInt32Var("x", 21),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = true}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
              }),
              .else_stmts = StmtListOf({
                  S(ReturnStmt{.value = E(IntLitExpr{.value = 0})}),
              }),
          }),
          S(ReturnStmt{.value = E(IntLitExpr{.value = 1})}),
      }),
  });

  static constexpr std::size_t kEntry = 0;
  static constexpr std::size_t kLast = 1;
  static constexpr std::size_t kAfterBranch = 2;
  static constexpr std::size_t kThen = 3;
  static constexpr std::size_t kElse = 4;

  ASSERT_THAT(states, SizeIs(5));

  EXPECT_THAT(states[kThen]->live_in, UnorderedElementsEqual(I("x")));
  EXPECT_THAT(states[kElse]->live_in, IsEmpty());
  EXPECT_THAT(states[kEntry]->live_in, IsEmpty());
  EXPECT_THAT(states[kAfterBranch]->live_in, IsEmpty());
  EXPECT_THAT(states[kLast]->live_in, IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, VariableReadAroundALoop) {
  const auto states = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          DeclareInt32Var("x", 21),
          S(LoopStmt{.stmts = StmtListOf({
                         S(IfStmt{
                             .cond = E(IdentExpr{.name = I("x")}),
                             .then_stmts = StmtListOf({S(BreakStmt{})}),
                             .else_stmts = StmtListOf({}),
                         }),
                     })}),
          S(ReturnStmt{.value = E(IntLitExpr{.value = 0})}),
      }),
  });

  static constexpr std::size_t kEntry = 0;
  static constexpr std::size_t kLast = 1;
  static constexpr std::size_t kAfterLoop = 2;
  static constexpr std::size_t kLoopHeader = 3;
  static constexpr std::size_t kLoopBody = 4;
  static constexpr std::size_t kBreak = 5;

  ASSERT_THAT(states, SizeIs(6));

  EXPECT_THAT(states[kLoopHeader]->live_in, UnorderedElementsEqual(I("x")));
  EXPECT_THAT(states[kLoopBody]->live_in, UnorderedElementsEqual(I("x")));
  EXPECT_THAT(states[kBreak]->live_in, IsEmpty());
  EXPECT_THAT(states[kAfterLoop]->live_in, IsEmpty());
  EXPECT_THAT(states[kLast]->live_in, IsEmpty());
  EXPECT_THAT(states[kEntry]->live_in, IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, ParameterReadAfterABranch) {
  const auto states = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({P(FuncParam{
          .name = I("a"),
          .type_constraint = T("Int32"),
      })}),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = true}),
              .then_stmts = StmtListOf({}),
              .else_stmts = StmtListOf({}),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("a")})}),
      }),
  });

  static constexpr std::size_t kEntry = 0;
  static constexpr std::size_t kLast = 1;
  static constexpr std::size_t kAfterBranch = 2;
  static constexpr std::size_t kThen = 3;

  ASSERT_THAT(states, SizeIs(4));

  EXPECT_THAT(states[kEntry]->live_in, UnorderedElementsEqual(I("a")));
  EXPECT_THAT(states[kThen]->live_in, UnorderedElementsEqual(I("a")));
  EXPECT_THAT(states[kAfterBranch]->live_in, UnorderedElementsEqual(I("a")));
  EXPECT_THAT(states[kLast]->live_in, IsEmpty());
}

}  // namespace
}  // namespace lucid
