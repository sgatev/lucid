#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <utility>

#include "lucid/core/functional/identity.h"
#include "lucid/core/hash/hash.h"

namespace lucid {

// A hash table that contains values of type `V`.
template <typename V, typename P = V,
          const P& (*Project)(const V&) = &identity<const V&>>
class HashTable {
 public:
  class Iterator {
   public:
    using value_type = V;

    bool operator==(const Iterator&) const = default;

    value_type& operator*() const { return *(table_->slot(pos_)); }

    value_type* operator->() const { return table_->slot(pos_); }

    Iterator& operator++() {
      pos_ = table_->next_full(pos_ + 1);
      return *this;
    }

    Iterator operator++(int) {
      Iterator it(*this);
      ++(*this);
      return it;
    }

   private:
    friend class HashTable;

    Iterator(const HashTable& table, std::size_t pos)
        : table_(&table), pos_(pos) {}

    const HashTable* table_;
    std::size_t pos_;
  };

  class ConstIterator {
   public:
    using value_type = const V;

    bool operator==(const ConstIterator&) const = default;

    value_type& operator*() const { return *(table_->slot(pos_)); }

    value_type* operator->() const { return table_->slot(pos_); }

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
        full_slots_count_(0),
        non_empty_slots_count_(0),
        storage_(alloc_storage(capacity)) {}

  HashTable(HashTable&& other) noexcept
      : capacity_mask_(other.capacity_mask_),
        full_slots_count_(other.full_slots_count_),
        non_empty_slots_count_(other.non_empty_slots_count_),
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
    full_slots_count_ = other.full_slots_count_;
    non_empty_slots_count_ = other.non_empty_slots_count_;
    std::swap(storage_, other.storage_);
    return *this;
  }

  bool operator==(const HashTable& other) const noexcept {
    return other.full_slots_count_ == full_slots_count_ &&
           std::all_of(other.begin(), other.end(), [&](const auto& val) {
             auto it = Find(Project(val));
             return it != end() && *it == val;
           });
  }

  // Returns an iterator that corresponds to the given projection or `end()`.
  inline Iterator Find(const P& proj) {
    return Iterator(*this, find_offset(proj));
  }

  // Returns a const iterator that corresponds to the given projection or
  // `end()`.
  inline ConstIterator Find(const P& proj) const {
    return ConstIterator(*this, find_offset(proj));
  }

  // Returns (slot, false) if the table contains a value that corresponds to
  // `proj`. Otherwise, allocates a new slot for a value and returns (slot,
  // true).
  template <typename... Ts>
  inline std::pair<V*, bool> FindOrAlloc(const P& proj) {
    if (non_empty_slots_count_ > (capacity() >> 1)) resize();

    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      std::uint8_t& offset_meta = *meta(offset);
      V* offset_slot = slot(offset);
      if (offset_meta == proj_meta && Project(*offset_slot) == proj) {
        return std::make_pair(offset_slot, false);
      }
      if (!full(offset_meta)) {
        if (empty(offset_meta)) ++non_empty_slots_count_;
        offset_meta = proj_meta;
        ++full_slots_count_;
        return std::make_pair(offset_slot, true);
      }
    }
  }

  // Inserts the given `val` and returns true if `Project(val)` is not already
  // inserted. Otherwise returns false.
  inline bool Insert(V val) {
    auto [val_slot, allocated] = FindOrAlloc(Project(val));
    if (allocated) new (val_slot) V(std::move(val));
    return allocated;
  }

  // Inserts the given `val` and returns true if `Project(val)` is not already
  // inserted. Otherwise overrides the value and returns false.
  inline bool Set(V val) {
    auto [val_slot, allocated] = FindOrAlloc(Project(val));
    if (allocated) {
      new (val_slot) V(std::move(val));
    } else {
      *val_slot = std::move(val);
    }
    return allocated;
  }

  // Removes the value that corresponds to the given projection and returns it
  // if present. Otherwise returns nullopt.
  inline std::optional<V> Remove(const P& proj) {
    const std::size_t offset = find_offset(proj);
    if (offset == capacity()) return std::nullopt;

    *meta(offset) = 0b11111110;
    --full_slots_count_;
    return std::move(*slot(offset));
  }

  // Returns the number of unique values inserted so far.
  inline std::size_t size() const noexcept { return full_slots_count_; }

  // Returns an iterator referring to the first value in the table or `end()`,
  // if there isn't one.
  Iterator begin() noexcept { return Iterator(*this, next_full(0)); }

  // Returns a const iterator referring to the first value in the table or
  // `end()`, if there isn't one.
  ConstIterator begin() const noexcept {
    return ConstIterator(*this, next_full(0));
  }

  // Returns an iterator past the last value in the table.
  Iterator end() noexcept { return Iterator(*this, capacity()); }

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
    while (pos < capacity() && !full(*meta(pos))) ++pos;
    return pos;
  }

  void resize() noexcept {
    *this = std::move(HashTable(capacity() << 1).with_content_from(*this));
  }

  // Returns an offset that corresponds to the given projection or `capacity()`.
  inline std::size_t find_offset(const P& proj) const {
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = proj_hash & 0b01111111;

    std::size_t offset = proj_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *meta(offset);
      if (offset_meta == proj_meta && Project(*slot(offset)) == proj) {
        return offset;
      }
      if (empty(offset_meta)) return capacity();
    }
  }

  template <typename T>
  inline HashTable& with_content_from(T&& other) noexcept {
    for (std::size_t pos = 0; pos < other.capacity(); ++pos) {
      const std::uint8_t proj_meta = *other.meta(pos);
      if (!full(proj_meta)) continue;

      V& value = *other.slot(pos);
      const P& proj = Project(value);
      const std::size_t proj_hash = Hash(proj);

      std::size_t offset = proj_hash;
      for (std::size_t i = 0;; ++i) {
        offset = (offset + i) & capacity_mask_;

        if (full(*meta(offset))) continue;

        *meta(offset) = proj_meta;
        new (slot(offset)) V(std::move(value));
        break;
      }
    }
    full_slots_count_ = other.full_slots_count_;
    non_empty_slots_count_ = other.non_empty_slots_count_;
    return *this;
  }

  inline std::uint8_t* meta(std::size_t i) const noexcept {
    return storage_ + i;
  }

  inline V* slot(std::size_t i) const noexcept {
    return reinterpret_cast<V*>(storage_ + capacity()) + i;
  }

  std::size_t capacity_mask_;
  std::size_t full_slots_count_;
  std::size_t non_empty_slots_count_;
  std::uint8_t* storage_;
};

}  // namespace lucid
