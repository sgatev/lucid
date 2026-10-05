#include "lucid/syntax/liveness.h"

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/string/index.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

class SyntaxLivenessAnalysisTest : public Test, public AstFixture {
 protected:
  // Returns the variables live where each block is entered, by block id.
  std::vector<std::vector<StringIndex::Ref>> AnalyzeLiveness(
      FuncDefStmt func_def) {
    auto syn_cfg = ::lucid::BuildControlFlowGraph(syn_ctx_, func_def);
    SyntaxLivenessAnalysis analysis(syn_ctx_, syn_cfg);
    return LiveIn(analysis, RunDataflow(Backward(syn_cfg), analysis));
  }

  std::vector<std::vector<StringIndex::Ref>> AnalyzeLiveness(
      FuncDefStmt func_def, const HashSet<StringIndex::Ref>& vars) {
    auto syn_cfg = ::lucid::BuildControlFlowGraph(syn_ctx_, func_def);
    SyntaxLivenessAnalysis analysis(syn_ctx_, syn_cfg, vars);
    return LiveIn(analysis, RunDataflow(Backward(syn_cfg), analysis));
  }

  static std::vector<std::vector<StringIndex::Ref>> LiveIn(
      const SyntaxLivenessAnalysis& analysis,
      const std::vector<std::optional<SyntaxLivenessAnalysis::State>>& states) {
    std::vector<std::vector<StringIndex::Ref>> live_in;
    live_in.reserve(states.size());
    for (const auto& state : states) {
      live_in.push_back(analysis.LiveIn(state.value()));
    }
    return live_in;
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

  EXPECT_THAT(AnalyzeLiveness(func_def), Elements(IsEmpty(), IsEmpty()));
}

TEST(SyntaxLivenessAnalysisTest, VariableReadInTheBlockThatWroteIt) {
  const auto live_in = AnalyzeLiveness(FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          DeclareInt32Var("x", 21),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ASSERT_THAT(live_in, SizeIs(2));

  EXPECT_THAT(live_in[0], IsEmpty());
  EXPECT_THAT(live_in[1], IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, VariableNeverRead) {
  const auto live_in = AnalyzeLiveness(FuncDefStmt{
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

  ASSERT_THAT(live_in, SizeIs(4));

  for (const auto& block_live_in : live_in) {
    EXPECT_THAT(block_live_in, IsEmpty());
  }
}

TEST(SyntaxLivenessAnalysisTest, VariableReadOnOneBranchOnly) {
  const auto live_in = AnalyzeLiveness(FuncDefStmt{
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

  ASSERT_THAT(live_in, SizeIs(5));

  EXPECT_THAT(live_in[kThen], UnorderedElementsEqual(I("x")));
  EXPECT_THAT(live_in[kElse], IsEmpty());
  EXPECT_THAT(live_in[kEntry], IsEmpty());
  EXPECT_THAT(live_in[kAfterBranch], IsEmpty());
  EXPECT_THAT(live_in[kLast], IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, VariableReadAroundALoop) {
  const auto live_in = AnalyzeLiveness(FuncDefStmt{
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

  ASSERT_THAT(live_in, SizeIs(6));

  EXPECT_THAT(live_in[kLoopHeader], UnorderedElementsEqual(I("x")));
  EXPECT_THAT(live_in[kLoopBody], UnorderedElementsEqual(I("x")));
  EXPECT_THAT(live_in[kBreak], IsEmpty());
  EXPECT_THAT(live_in[kAfterLoop], IsEmpty());
  EXPECT_THAT(live_in[kLast], IsEmpty());
  EXPECT_THAT(live_in[kEntry], IsEmpty());
}

TEST(SyntaxLivenessAnalysisTest, ParameterReadAfterABranch) {
  const auto live_in = AnalyzeLiveness(FuncDefStmt{
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

  ASSERT_THAT(live_in, SizeIs(4));

  EXPECT_THAT(live_in[kEntry], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kThen], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kAfterBranch], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kLast], IsEmpty());
}

// Two parameters read after a branch, of which only one is asked about: the
// other is live in all the same places, but is never taken to be.
TEST(SyntaxLivenessAnalysisTest, OnlyTheVariablesAskedAbout) {
  HashSet<StringIndex::Ref> vars;
  vars.Insert(I("a"));
  const auto live_in = AnalyzeLiveness(
      FuncDefStmt{
          .name = I("foo"),
          .params = ParamListOf({
              P(FuncParam{.name = I("a"), .type_constraint = T("Int32")}),
              P(FuncParam{.name = I("b"), .type_constraint = T("Int32")}),
          }),
          .result_type = T("Int32"),
          .stmts = StmtListOf({
              S(IfStmt{
                  .cond = E(BoolLitExpr{.value = true}),
                  .then_stmts = StmtListOf({}),
                  .else_stmts = StmtListOf({}),
              }),
              S(ReturnStmt{.value = E(BinaryOpExpr{
                               .op = BinaryOp::Add,
                               .lhs = E(IdentExpr{.name = I("a")}),
                               .rhs = E(IdentExpr{.name = I("b")}),
                           })}),
          }),
      },
      vars);

  static constexpr std::size_t kEntry = 0;
  static constexpr std::size_t kLast = 1;
  static constexpr std::size_t kAfterBranch = 2;
  static constexpr std::size_t kThen = 3;

  ASSERT_THAT(live_in, SizeIs(4));

  EXPECT_THAT(live_in[kEntry], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kThen], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kAfterBranch], UnorderedElementsEqual(I("a")));
  EXPECT_THAT(live_in[kLast], IsEmpty());
}

}  // namespace
}  // namespace lucid
