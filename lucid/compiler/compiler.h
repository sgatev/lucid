#include <expected>
#include <filesystem>
#include <string_view>
#include <variant>

#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/type.h"

namespace lucid {

template <typename... Ts>
using CompositeError = std::variant<Ts...>;

std::expected<std::vector<Def>, ParserError> ParseDefs(std::string_view src,
                                                       SyntaxContext& ctx);

struct CompileConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

using CompileError =
    CompositeError<ReadFileError, ParserError, TypeError, CompError>;

std::expected<void, CompileError> CompileCode(const CompileConfig& config);

struct BuildConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

using BuildError =
    CompositeError<ReadFileError, ParserError, TypeError, CompError>;

std::expected<void, BuildError> BuildCode(BuildConfig config);

}  // namespace lucid
