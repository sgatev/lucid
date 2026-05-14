#include "lucid/syntax/liveness.h"

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

class SyntaxLivenessAnalysisTest : public testing::Test, public AstFixture {
 protected:
  using State = SyntaxLivenessAnalysis::State;

  std::vector<std::optional<State>> AnalyzeReachability(FuncDefStmt func_def) {
    auto syn_cfg = ::lucid::BuildControlFlowGraph(syn_ctx_, func_def);
    SyntaxLivenessAnalysis analysis(syn_ctx_, syn_cfg);
    return RunDataflow(Forward(syn_cfg), analysis);
  }
};

TEST_F(SyntaxLivenessAnalysisTest, EmptyFunc) {
  auto func_def = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
  };

  EXPECT_THAT(AnalyzeReachability(func_def),
              ElementsAreArray({
                  Optional(AllOf(Field(&State::live_in, IsEmpty()),
                                 Field(&State::live_out, IsEmpty()))),

                  Optional(AllOf(Field(&State::live_in, IsEmpty()),
                                 Field(&State::live_out, IsEmpty()))),
              }));
}

}  // namespace
}  // namespace lucid
