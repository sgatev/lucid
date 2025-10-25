#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace lucid {

// A container of values that can be added dynamically and retrieved via
// references.
template <typename T>
class Arena {
 public:
  // The type of references for values in the arena.
  using Ref = std::uint32_t;

  // The null arena reference.
  static constexpr Ref kNullRef = std::numeric_limits<Ref>::max();

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
    indices_.push_back(indices_[ref]);
    return indices_.size() - 1;
  }

  // Returns the value associated with `ref` in the arena.
  //
  // Requires:
  // - `ref` must not be `kNullRef`.
  const T& Get(Ref ref) const { return values_[indices_[ref]]; }
  T& Get(Ref ref) { return values_[indices_[ref]]; }

  // Returns true iff `lhs` and `rhs` refer to the same value.
  bool Equiv(Ref lhs, Ref rhs) const { return indices_[lhs] == indices_[rhs]; }

  // Returns the number of values that were added to the arena.
  std::size_t Size() const { return values_.size(); }

  // Returns an iterator to the first value in the arena.
  auto begin() { return values_.begin(); }
  auto begin() const { return values_.begin(); }

  // Returns an iterator following the last value in the arena.
  auto end() { return values_.end(); }
  auto end() const { return values_.end(); }

 private:
  std::vector<std::size_t> indices_;
  std::vector<T> values_;
};

// A reference in an arena of type `Arena<T>`.
template <typename T>
using ArenaRef = typename Arena<T>::Ref;

}  // namespace lucid
