#include "lucid/syntax/ast_printer.h"

#include <format>
#include <sstream>

#include "lucid/core/cli/format.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"

namespace lucid {
namespace {

class AstPrinterTest : public Test, public AstFixture {};

// The label is colored, not the indentation in front of it.
TEST(AstPrinterTest, ColorsLabelAfterIndentation) {
  const ExprRef value = E(IntLitExpr{.value = 0});
  const StmtRef ret = S(ReturnStmt{.value = value});

  std::stringstream out;
  EnableColor(out);
  PrintStmt(syn_ctx_, ret, out);

  EXPECT_EQ(out.str(), std::format("\33[34mS{}: \33[mReturnStmt {{\n"
                                   "  .value = {{\n"
                                   "    \33[34mE{}: \33[mIntLitExpr {{\n"
                                   "      .value = 0\n"
                                   "    }}\n"
                                   "  }}\n"
                                   "}}\n",
                                   ret.id(), value.id()));
}

}  // namespace
}  // namespace lucid
