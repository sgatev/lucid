#include "lucid/core/string/concat.h"

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string_view>

namespace lucid {

std::string Concat(std::span<const std::string_view> parts,
                   std::string_view glue) {
  std::string res;
  if (parts.empty()) return res;

  const std::size_t size =
      std::ranges::fold_left(
          parts | std::views::transform(&std::string_view::size), 0,
          std::plus()) +
      (parts.size() - 1) * glue.size();
  res.reserve(size);

  for (std::string_view part : parts) {
    if (!res.empty()) res.append_range(glue);
    res.append_range(part);
  }

  return res;
}

}  // namespace lucid
