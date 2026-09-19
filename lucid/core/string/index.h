#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "lucid/core/container/hash_map.h"

namespace lucid {

// Maps strings to small references that can be used to identify them.
class StringIndex {
 public:
  struct Ref {
   public:
    bool operator==(const Ref& other) const = default;

    bool operator!=(const Ref& other) const { return !(*this == other); }

   private:
    friend class StringIndex;
    friend std::size_t Hash(const Ref&);

    explicit Ref(std::uint32_t begin) : begin_(begin) {}

    std::uint32_t begin_;
  };

  // Returns a reference that identifies the given string.
  Ref ref(std::string_view s) {
    if (auto res = string_to_ref_.Get(s); res.has_value()) return *res;

    Ref ref(ref_to_string_.size());
    ref_to_string_.push_back(s);
    string_to_ref_.Insert(s, ref);

    return ref;
  }

  // Returns the string identified by the given reference.
  std::string_view deref(Ref ref) const { return ref_to_string_[ref.begin_]; }

 private:
  HashMap<std::string_view, Ref> string_to_ref_;
  std::vector<std::string_view> ref_to_string_;
};

inline std::size_t Hash(const lucid::StringIndex::Ref& v) {
  return Hash(v.begin_);
}

}  // namespace lucid
