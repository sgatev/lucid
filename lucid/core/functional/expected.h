#pragma once

namespace lucid {

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

#define UNIQUE_VAR(prefix) CONCAT(prefix##_, __LINE__)

#define ASSIGN_OR_RETURN(dst, src)                   \
  auto UNIQUE_VAR(aor) = src;                        \
  if (!UNIQUE_VAR(aor).has_value()) [[unlikely]]     \
    return std::unexpected(UNIQUE_VAR(aor).error()); \
  dst = std::move(UNIQUE_VAR(aor)).value()  // NOLINT

#define RETURN_IF_ERROR(src)                          \
  if (auto dst = (src); dst.has_value()) [[likely]] { \
  } else                                              \
    return std::unexpected(dst.error())

}  // namespace lucid
