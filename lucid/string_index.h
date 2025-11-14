#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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

    Ref(std::uint32_t begin, std::uint32_t size) : begin_(begin), size_(size) {}

    std::uint32_t begin_;
    std::uint32_t size_;
  };

  // Returns a reference that identifies the given string.
  Ref ref(std::string_view s) {
    auto pos = data_.find(s);
    if (pos == std::string::npos) {
      pos = data_.size();
      data_.append(s);
    }
    return Ref(pos, s.size());
  }

  // Returns a unique reference.
  Ref ref() {
    auto pos = data_.size();
    data_.append("x");
    return Ref(pos, 1);
  }

  // Returns the string identified by the given reference.
  std::string_view deref(Ref ref) const {
    return std::string_view(data_.data() + ref.begin_, ref.size_);
  }

 private:
  std::string data_;
};

}  // namespace lucid

namespace std {

template <>
struct hash<lucid::StringIndex::Ref> {
  size_t operator()(const lucid::StringIndex::Ref& ref) const {
    std::size_t h1 = hash<uint32_t>()(ref.begin_);
    std::size_t h2 = hash<uint32_t>()(ref.size_);
    return h1 ^ (h2 << 1);
  }
};

}  // namespace std
