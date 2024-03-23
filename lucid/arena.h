#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace lucid {

// Stores values and provides access to them via references.
template <typename T>
class Arena {
 public:
  using Ref = std::uint32_t;

  // A null `Arena<T>` reference.
  static constexpr Ref kNullRef = std::numeric_limits<std::uint32_t>::max();

  Arena() = default;
  Arena(Arena&&) = default;
  Arena& operator=(Arena&&) = default;

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  // Adds `value` to the arena and returns a reference that can be used to
  // retrieve it.
  Ref add(T value) {
    Ref ref = indices_.size();
    indices_.push_back(values_.size());
    values_.push_back(std::move(value));
    return ref;
  }

  // Adds another reference for the value that `ref` refers to.
  Ref alias(Ref ref) {
    indices_.push_back(indices_[ref]);
    return indices_.size() - 1;
  }

  // Returns the value associated with `ref` in the arena.
  //
  // Requirements:
  // - `ref` must not be `kNullRef`.
  const T& get(Ref ref) const { return values_[indices_[ref]]; }
  T& get(Ref ref) { return values_[indices_[ref]]; }

  // Returns true iff the two references refer to the same value.
  bool equiv(Ref lhs, Ref rhs) const { return indices_[lhs] == indices_[rhs]; }

  // Returns the number of values that were added to the arena.
  std::size_t size() const { return values_.size(); }

  // Returns an iterator to the first value of the arena.
  auto begin() const { return values_.begin(); }

  // Returns an iterator to the value following the last value of the arena.
  auto end() const { return values_.end(); }

 private:
  std::vector<std::size_t> indices_;
  std::vector<T> values_;
};

// A reference in an arena of type `Arena<T>`.
template <typename T>
using ArenaRef = typename Arena<T>::Ref;

}  // namespace lucid
