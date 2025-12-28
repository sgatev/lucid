#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "lucid/hash.h"
#include "lucid/optional_ref.h"

namespace lucid {

template <typename T>
const T& Identity(const T& i) noexcept {
  return i;
}

// A hash table that contains values of type `V`.
template <typename V, typename P = V, const P& (*Project)(const V&) = &Identity>
class HashTable {
 public:
  HashTable() : capacity_mask_(kInitialCapacity - 1), size_(0) {
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        alignof(std::max_align_t), capacity() + capacity() * sizeof(V)));
    std::fill(meta(), meta() + capacity(), 0);
  }

  HashTable(HashTable&& other) {
    capacity_mask_ = other.capacity_mask_;
    size_ = other.size_;
    storage_ = std::exchange(other.storage_, nullptr);
  }

  HashTable(const HashTable& other) {
    capacity_mask_ = other.capacity_mask_;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        alignof(std::max_align_t), capacity() + capacity() * sizeof(V)));
    std::fill(meta(), meta() + capacity(), 0);

    for (std::size_t pos = 0; pos < other.capacity(); ++pos) {
      if (*(other.meta() + pos) != 0) Insert(*(other.slots() + pos));
    }
  }

  ~HashTable() {
    if (storage_ != nullptr) std::free(storage_);
  }

  // Returns the value that corresponds to the given projection or nullopt.
  inline OptionalRef<V> Find(const P& proj) const {
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash | 0b10000000;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == proj_meta) {
        if (V* value = slots() + offset; Project(*value) == proj) return *value;
      }
      if (offset_meta == 0) return std::nullopt;
    }
  }

  // Inserts the given `value` and returns true if `Project(value)` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(V value) {
    if (size_ > (capacity() >> 1)) Resize();

    const std::size_t proj_hash = Hash(Project(value));
    const std::uint8_t proj_meta = proj_hash | 0b10000000;
    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      if (std::uint8_t offset_meta = *(meta() + offset); offset_meta == 0) {
        *(meta() + offset) = proj_meta;
        *(slots() + offset) = std::move(value);
        ++size_;
        return true;
      }
      if (Project(*(slots() + offset)) == Project(value)) return false;
    }
  }

  // Returns the number of unique values inserted so far.
  inline std::size_t Size() const { return size_; }

 private:
  static constexpr std::size_t kInitialCapacity = [] {
    static constexpr std::size_t max_align = alignof(std::max_align_t);
    return ((64 + max_align - 1) / max_align) * max_align;
  }();

  inline std::size_t capacity() const { return capacity_mask_ + 1; }

  inline void Resize() {
    const std::size_t old_capacity = capacity();
    std::uint8_t* old_meta = meta();
    V* old_slots = slots();

    capacity_mask_ = (old_capacity << 1) - 1;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        alignof(std::max_align_t), capacity() + capacity() * sizeof(V)));
    std::fill(meta(), meta() + capacity(), 0);

    for (std::size_t pos = 0; pos < old_capacity; ++pos) {
      if (*(old_meta + pos) != 0) Insert(std::move(*(old_slots + pos)));
    }

    std::free(old_meta);
  }

  inline std::uint8_t* meta() const { return storage_; }

  inline V* slots() const {
    return reinterpret_cast<V*>(storage_ + capacity());
  }

  std::size_t capacity_mask_;
  std::size_t size_;
  std::uint8_t* storage_ = nullptr;
};

}  // namespace lucid
