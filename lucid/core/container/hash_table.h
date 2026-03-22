#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <utility>

#include "lucid/core/container/optional_ref.h"
#include "lucid/core/functional/identity.h"
#include "lucid/core/hash/hash.h"

namespace lucid {

// A hash table that contains values of type `V`.
template <typename V, typename P = V,
          const P& (*Project)(const V&) = &identity<const V&>>
class HashTable {
 public:
  class ConstIterator {
   public:
    using value_type = const V;

    bool operator==(const ConstIterator&) const = default;

    const V& operator*() const { return *(table_->slots() + pos_); }

    const V* operator->() const { return table_->slots() + pos_; }

    ConstIterator& operator++() {
      pos_ = table_->next_full(pos_ + 1);
      return *this;
    }

    ConstIterator operator++(int) {
      ConstIterator it(*this);
      ++(*this);
      return it;
    }

   private:
    friend class HashTable;

    ConstIterator(const HashTable& table, std::size_t pos)
        : table_(&table), pos_(pos) {}

    const HashTable* table_;
    std::size_t pos_;
  };

  HashTable() : HashTable(kInitialCapacity) {}

  HashTable(std::size_t capacity)
      : capacity_mask_(capacity - 1),
        size_(0),
        storage_(alloc_storage(capacity)) {}

  HashTable(HashTable&& other)
      : capacity_mask_(other.capacity_mask_),
        size_(other.size_),
        storage_(std::exchange(other.storage_, nullptr)) {}

  HashTable(const HashTable& other)
      : capacity_mask_(other.capacity_mask_),
        storage_(alloc_storage(capacity())) {
    with_content_from(other);
  }

  ~HashTable() {
    if (storage_ != nullptr) std::free(storage_);
  }

  HashTable& operator=(HashTable other) {
    capacity_mask_ = other.capacity_mask_;
    size_ = other.size_;
    std::swap(storage_, other.storage_);
    return *this;
  }

  bool operator==(const HashTable& other) const noexcept {
    if (size_ != other.size_) return false;
    for (std::uint8_t offset = 0; offset < other.capacity(); ++offset) {
      if (!full(*(other.meta() + offset))) continue;

      const V& other_value = *(other.slots() + offset);
      OptionalRef<V> value = Find(Project(other_value));
      if (value != other_value) return false;
    }
    return true;
  }

  // Returns the value that corresponds to the given projection or nullopt.
  inline OptionalRef<V> Find(const P& proj) const {
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == proj_meta) {
        V& value = *(slots() + offset);
        if (Project(value) == proj) return value;
      }
      if (empty(offset_meta)) return std::nullopt;
    }
  }

  // Inserts the given `value` and returns true if `Project(value)` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(V value) {
    if (size_ > (capacity() >> 1)) resize();

    const P& proj = Project(value);
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == proj_meta && Project(*(slots() + offset)) == proj) {
        return false;
      }
      if (!full(offset_meta)) {
        *(meta() + offset) = proj_meta;
        new (slots() + offset) V(std::move(value));
        ++size_;
        return true;
      }
    }
  }

  // Inserts the given `value` and returns true if `Project(value)` is not
  // already inserted. Otherwise overrides the value and returns false.
  inline bool Set(V value) {
    if (size_ > (capacity() >> 1)) resize();

    const P& proj = Project(value);
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == proj_meta && Project(*(slots() + offset)) == proj) {
        *(slots() + offset) = std::move(value);
        return false;
      }
      if (!full(offset_meta)) {
        *(meta() + offset) = proj_meta;
        *(slots() + offset) = std::move(value);
        ++size_;
        return true;
      }
    }
  }

  // Removes the value that corresponds to the given projection and returns true
  // if present. Otherwise returns false.
  inline bool Remove(const P& proj) {
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == proj_meta && Project(*(slots() + offset)) == proj) {
        *(meta() + offset) = 0b11111110;
        --size_;
        return true;
      }
      if (empty(offset_meta)) return false;
    }
  }

  // Returns the number of unique values inserted so far.
  inline std::size_t size() const noexcept { return size_; }

  // Returns a const iterator referring to the first value in the table or
  // `end()`, if there isn't one.
  ConstIterator begin() const noexcept {
    return ConstIterator(*this, next_full(0));
  }

  // Returns a const iterator past the last value in the table.
  ConstIterator end() const noexcept {
    return ConstIterator(*this, capacity());
  }

 private:
  static constexpr std::size_t kInitialCapacity = [] {
    static constexpr std::size_t max_align = alignof(std::max_align_t);
    return ((64 + max_align - 1) / max_align) * max_align;
  }();

  static constexpr std::uint8_t* alloc_storage(std::size_t size) noexcept {
    auto* storage =
        static_cast<std::uint8_t*>(std::malloc(size + size * sizeof(V)));
    std::fill_n(storage, size, 0b10000000);
    return storage;
  }

  static constexpr bool full(std::uint8_t meta) noexcept {
    return (meta & 0b10000000) == 0;
  }

  static constexpr bool empty(std::uint8_t meta) noexcept {
    return meta == 0b10000000;
  }

  inline std::size_t capacity() const noexcept { return capacity_mask_ + 1; }

  inline std::size_t next_full(std::size_t pos) const noexcept {
    while (pos < capacity() && !full(*(meta() + pos))) ++pos;
    return pos;
  }

  void resize() noexcept {
    *this = std::move(HashTable(capacity() << 1).with_content_from(*this));
  }

  template <typename T>
  inline HashTable& with_content_from(T&& other) noexcept {
    for (std::size_t pos = 0; pos < other.capacity(); ++pos) {
      const std::uint8_t proj_meta = *(other.meta() + pos);
      if (!full(proj_meta)) continue;

      V& value = *(other.slots() + pos);
      const P& proj = Project(value);
      const std::size_t proj_hash = Hash(proj);

      std::size_t offset = proj_hash;
      for (std::size_t i = 0;; ++i) {
        offset = (offset + i) & capacity_mask_;

        if (full(*(meta() + offset))) continue;

        *(meta() + offset) = proj_meta;
        new (slots() + offset) V(std::move(value));
        break;
      }
    }
    size_ = other.size_;
    return *this;
  }

  inline std::uint8_t* meta() const noexcept { return storage_; }

  inline V* slots() const noexcept {
    return reinterpret_cast<V*>(storage_ + capacity());
  }

  std::size_t capacity_mask_;
  std::size_t size_;
  std::uint8_t* storage_;
};

}  // namespace lucid
