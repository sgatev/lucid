#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>

namespace lucid {

// A container of values that can be added dynamically and retrieved via
// references.
template <typename T>
class Arena {
 public:
  // The type of references for values in the arena.
  struct Ref {
   public:
    constexpr Ref(std::uint32_t id) : id_(id) {}

    constexpr Ref() : Ref(std::numeric_limits<std::uint32_t>::max()) {}

    bool operator==(const Ref& other) const { return id_ == other.id_; }

    bool operator!=(const Ref& other) const { return id_ != other.id_; }

    Ref operator+(std::uint32_t offset) const { return Ref(id_ + offset); }

    Ref operator-(std::uint32_t offset) const { return Ref(id_ - offset); }

    Ref& operator++() {
      ++id_;
      return *this;
    }

    Ref& operator--() {
      --id_;
      return *this;
    }

    std::uint32_t id() const { return id_; }

    friend std::ostream& operator<<(std::ostream& os, const Ref& ref) {
      return os << ref.id_;
    }

   private:
    std::uint32_t id_;
  };

  // The null arena reference.
  static constexpr Ref kNullRef;

  // Adds `value` to the arena and returns a reference that can be used to
  // retrieve it.
  Ref Add(T value) {
    Ref ref = indices_.size();
    indices_.push_back(values_.size());
    values_.push_back(std::move(value));
    return ref;
  }

  // Adds another reference for the value that `ref` refers to.
  Ref Alias(Ref ref) {
    indices_.push_back(indices_[ref.id()]);
    return indices_.size() - 1;
  }

  // Returns the value associated with `ref` in the arena.
  //
  // Requires:
  // - `ref` must not be `kNullRef`.
  auto& Get(this auto&& self, Ref ref) {
    return self.values_[self.indices_[ref.id()]];
  }

  // Returns true iff `lhs` and `rhs` refer to the same value.
  bool Equiv(Ref lhs, Ref rhs) const {
    return indices_[lhs.id()] == indices_[rhs.id()];
  }

  // Returns the number of values that were added to the arena.
  std::size_t Size() const { return values_.size(); }

  // Returns an iterator to the first value in the arena.
  auto begin(this auto&& self) { return self.values_.begin(); }

  // Returns an iterator following the last value in the arena.
  auto end(this auto&& self) { return self.values_.end(); }

 private:
  std::vector<std::size_t> indices_;
  std::vector<T> values_;
};

}  // namespace lucid
