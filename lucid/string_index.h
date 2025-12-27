#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "lucid/hash_map.h"
#include "lucid/optional_ref.h"

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
    friend struct std::hash<Ref>;
    friend std::size_t Hash(const Ref&);

    Ref(std::uint32_t begin, std::int32_t size) : begin_(begin), size_(size) {}

    std::uint32_t begin_;
    std::int32_t size_;
  };

  // Returns a reference that identifies the given string.
  Ref ref(std::string_view s) {
    OptionalRef<Ref> res = string_to_ref_.Find(s);
    if (res.has_value()) return *res;

    Ref ref(ref_to_string_.Size(), static_cast<std::int32_t>(s.size()));
    ref_to_string_.Insert(ref, s);
    string_to_ref_.Insert(s, ref);

    return ref;
  }

  // Returns a unique reference.
  Ref ref() { return Ref(unique_ident_++, -1); }

  // Returns the string identified by the given reference.
  std::string_view deref(Ref ref) const {
    if (ref.size_ < 0) return "<unique>";
    OptionalRef<std::string_view> res = ref_to_string_.Find(ref);
    assert(res.has_value());
    return *res;
  }

 private:
  HashMap<std::string_view, Ref> string_to_ref_;
  HashMap<Ref, std::string_view> ref_to_string_;
  std::uint32_t unique_ident_ = 0;
};

inline std::size_t Hash(const lucid::StringIndex::Ref& v) {
  return HashCombine(Hash(v.begin_), Hash(v.size_));
}

}  // namespace lucid

namespace std {

template <>
struct hash<lucid::StringIndex::Ref> {
  size_t operator()(const lucid::StringIndex::Ref& ref) const {
    std::size_t h1 = hash<uint32_t>()(ref.begin_);
    std::size_t h2 = hash<int32_t>()(ref.size_);
    return h1 ^ (h2 << 1);
  }
};

}  // namespace std
