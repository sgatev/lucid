#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/writer.h"

namespace lucid {

// Builds a string piece by piece.
class StringBuilder {
 public:
  // Appends `piece` to the end of the resulting string.
  void Append(std::string_view piece) {
    pieces_.push_back(piece);
    size_ += piece.size();
  }

  // Returns the string assembled through calls to `Append`.
  std::string Build() && {
    std::string result;
    result.reserve(size_);
    for (auto piece : pieces_) result.append(piece);
    return result;
  }

  // Writes the string assembled through calls to `Append` using `output`.
  void Write(Writer output) && {
    for (auto piece : pieces_) output(piece);
  }

 private:
  std::vector<std::string_view> pieces_;
  std::size_t size_ = 0;
};

}  // namespace lucid
