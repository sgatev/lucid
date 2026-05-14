#include <filesystem>
#include <string_view>

#include "lucid/core/functional/result.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/type.h"

namespace lucid {

Result<std::vector<FuncDefStmt>, ParserError> ParseFuncDefs(
    std::string_view src, SyntaxContext& ctx);

struct CompileConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

Result<void, ReadFileError, ParserError, TypeError, CompError> CompileCode(
    CompileConfig config);

struct BuildConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

Result<void, ReadFileError, ParserError, TypeError, CompError> BuildCode(
    BuildConfig config);

}  // namespace lucid
