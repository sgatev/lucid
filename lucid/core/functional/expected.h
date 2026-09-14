#pragma once

#include "lucid/core/meta/macros.h"

namespace lucid {

#define ASSIGN_OR_RETURN(dst, src)                         \
  auto LUCID_UNIQUE_VAR(aor) = src;                        \
  if (!LUCID_UNIQUE_VAR(aor).has_value()) [[unlikely]]     \
    return std::unexpected(LUCID_UNIQUE_VAR(aor).error()); \
  dst = std::move(LUCID_UNIQUE_VAR(aor)).value()  // NOLINT

#define RETURN_IF_ERROR(src)                          \
  if (auto dst = (src); dst.has_value()) [[likely]] { \
  } else                                              \
    return std::unexpected(dst.error())

}  // namespace lucid
