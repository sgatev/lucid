#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace lucid {

// Builds a string piece by piece.
class StringBuilder {
 public:
  // Appends `piece` to the end of the resulting string.
  void Append(std::string_view piece) { result_.append(piece); }

  // Returns the string assembled through calls to `Append`.
  std::string Build() && { return std::move(result_); }

 private:
  std::string result_;
};

}  // namespace lucid
