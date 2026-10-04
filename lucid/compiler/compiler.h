#include <expected>
#include <filesystem>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/context.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/scope.h"
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

using CompileError = CompositeError<ReadFileError, ParserError, ScopeError,
                                    TypeError, CompError>;

// Compiles the program `src` and writes it to `out` as an object file.
//
// Requires:
// - `src` must end with a zero byte, which is where the lexer stops.
std::expected<void, CompileError> CompileSource(std::string_view src,
                                                std::ostream& out);

std::expected<void, CompileError> CompileCode(const CompileConfig& config);

struct BuildConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

using BuildError = CompositeError<ReadFileError, ParserError, ScopeError,
                                  TypeError, CompError>;

std::expected<void, BuildError> BuildCode(BuildConfig config);

}  // namespace lucid
