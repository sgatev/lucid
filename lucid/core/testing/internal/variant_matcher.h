#pragma once

#include <string>
#include <variant>

namespace lucid::internal {

template <typename T, typename M>
class VariantMatcher {
 public:
  explicit VariantMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return "";
    return "variant with " + value_matcher_.DescribeActual(*actual);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return false;
    return value_matcher_.Matches(*actual);
  }

 private:
  M value_matcher_;
};

}  // namespace lucid::internal
