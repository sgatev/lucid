#pragma once

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace lucid {

// Stores values and provides access to them via references.
template <typename T>
class Arena {
 public:
  using Ref = std::size_t;

  // A null `Arena<T>` reference.
  static constexpr Ref kNullRef = std::numeric_limits<std::size_t>::max();

  Arena() = default;
  Arena(Arena&&) = default;
  Arena& operator=(Arena&&) = default;

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  // Adds `value` to the arena and returns a reference that can be used to
  // retrieve it.
  Ref add(T value) {
    values_.push_back(std::move(value));
    return values_.size() - 1;
  }

  // Returns the value associated with `ref` in the arena.
  //
  // `ref` must not be `kNullRef`.
  const T& get(Ref ref) const { return values_[ref]; }
  T& get(Ref ref) { return values_[ref]; }

 private:
  std::vector<T> values_;
};

// A reference in an arena of type `Arena<T>`.
template <typename T>
using ArenaRef = typename Arena<T>::Ref;

}  // namespace lucid
