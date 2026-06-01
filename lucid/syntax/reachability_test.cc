#include "lucid/syntax/reachability.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::Pair;
using ::testing::UnorderedElementsAreArray;
using ::testing::VariantWith;

class SyntaxReachabilityAnalysisTest : public testing::Test, public AstFixture {
 protected:
  using State = SyntaxReachabilityAnalysis::State;

  std::vector<std::optional<State>> AnalyzeReachability(FuncDefStmt func_def) {
    auto syn_cfg = ::lucid::BuildControlFlowGraph(syn_ctx_, func_def);
    SyntaxReachabilityAnalysis analysis(syn_cfg, syn_ctx_);
    return RunDataflow(Forward(syn_cfg), analysis);
  }
};

TEST_F(SyntaxReachabilityAnalysisTest, EmptyFunc) {
  auto func_def = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
  };

  EXPECT_THAT(AnalyzeReachability(func_def),
              ElementsAreArray({
                  Optional(AllOf(Field(&State::vars_in, IsEmpty()),
                                 Field(&State::vars_out, IsEmpty()))),

                  Optional(AllOf(Field(&State::vars_in, IsEmpty()),
                                 Field(&State::vars_out, IsEmpty()))),
              }));
}

TEST_F(SyntaxReachabilityAnalysisTest, Param) {
  auto x_name = I("x");
  auto x_ref = P(FuncParam{
      .name = x_name,
      .type_constraint = T("Int64"),
  });

  auto func_def = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          x_ref,
      }),
      .result_type = T("Void"),
  };

  EXPECT_THAT(
      AnalyzeReachability(func_def),
      ElementsAreArray({
          Optional(AllOf(
              Field(&State::vars_in,
                    UnorderedElementsAreArray({
                        Pair(x_name, VariantWith<ParamRef>(EquivTo(x_ref))),
                    })),
              Field(&State::vars_out,
                    UnorderedElementsAreArray({
                        Pair(x_name, VariantWith<ParamRef>(EquivTo(x_ref))),
                    })))),

          Optional(AllOf(
              Field(&State::vars_in,
                    UnorderedElementsAreArray({
                        Pair(x_name, VariantWith<ParamRef>(EquivTo(x_ref))),
                    })),
              Field(&State::vars_out,
                    UnorderedElementsAreArray({
                        Pair(x_name, VariantWith<ParamRef>(EquivTo(x_ref))),
                    })))),
      }));
}

}  // namespace
}  // namespace lucid
