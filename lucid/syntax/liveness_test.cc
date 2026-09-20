#include "lucid/syntax/liveness.h"

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

}  // namespace
}  // namespace lucid
