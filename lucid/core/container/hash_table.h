#pragma once

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <type_traits>
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

  explicit HashTable(std::size_t wanted_capacity)
      : capacity_mask_(std::bit_ceil(wanted_capacity) - 1),
        full_slots_count_(0),
        non_empty_slots_count_(0),
        storage_(alloc_storage(capacity())) {}

  HashTable(HashTable&& other) noexcept
      : capacity_mask_(other.capacity_mask_),
        full_slots_count_(other.full_slots_count_),
        non_empty_slots_count_(other.non_empty_slots_count_),
        storage_(std::exchange(other.storage_, nullptr)) {}

  HashTable(const HashTable& other)
      : capacity_mask_(other.capacity_mask_),
        full_slots_count_(other.full_slots_count_),
        non_empty_slots_count_(other.non_empty_slots_count_),
        storage_(alloc_storage(capacity())) {
    // The copy has the capacity of the original, so every value belongs in the
    // slot it came from and the meta bytes carry over whole. This keeps the
    // probe chains and the slots emptied by a removal as they were.
    std::memcpy(storage_, other.storage_, capacity());
    for (std::size_t pos = 0; pos < capacity(); ++pos) {
      if (full(*meta(pos))) new (slot(pos)) V(*other.slot(pos));
    }
  }

  ~HashTable() {
    if (storage_ == nullptr) return;

    destroy_values();
    std::free(storage_);
  }

  // Takes over the content of `other`, which is left holding what this table
  // had and destroys it in turn.
  //
  // Every member is swapped rather than only the storage, so that what `other`
  // is left with describes the storage it is left with: a table whose capacity
  // says more than its storage holds would be walked past its end.
  HashTable& operator=(HashTable other) {
    std::swap(capacity_mask_, other.capacity_mask_);
    std::swap(full_slots_count_, other.full_slots_count_);
    std::swap(non_empty_slots_count_, other.non_empty_slots_count_);
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
    if (non_empty_slots_count_ > max_load()) rehash();

    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = hash_meta(proj_hash);

    std::size_t first_free_offset = capacity();
    std::size_t offset = group_of(proj_hash);
    for (std::size_t step = 1;; ++step) {
      const std::uint64_t group_meta = group(offset);

      for (std::uint64_t matches = match(group_meta, proj_meta); matches != 0;
           matches &= matches - 1) {
        const std::size_t match_offset = first_match(offset, matches);
        if (Project(*slot(match_offset)) == proj) {
          return std::make_pair(slot(match_offset), false);
        }
      }

      // The first slot of the chain holding nothing, which is where the value
      // goes if the search runs out without finding `proj`. A slot holding
      // nothing is one that carries a high bit, whether it was never filled
      // or a removal emptied it.
      if (first_free_offset == capacity()) {
        if (const std::uint64_t frees = group_meta & kMetaHighs; frees != 0) {
          first_free_offset = first_match(offset, frees);
        }
      }

      // An empty slot is the end of the chain, so `proj` is not here.
      if (match(group_meta, kEmptyMeta) != 0) break;

      offset = next_group(offset, step);
    }

    ++full_slots_count_;
    if (empty(*meta(first_free_offset))) ++non_empty_slots_count_;
    *meta(first_free_offset) = proj_meta;
    return std::make_pair(slot(first_free_offset), true);
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

    *meta(offset) = kRemovedMeta;
    --full_slots_count_;

    // The slot holds no value once it is emptied, so what is left of the one
    // moved out of it is destroyed here rather than waiting for a later
    // insert to place a value over it.
    std::optional<V> value = std::move(*slot(offset));
    slot(offset)->~V();
    return value;
  }

  // Returns the number of unique values inserted so far.
  inline std::size_t size() const noexcept { return full_slots_count_; }

  // Returns the number of slots the table holds.
  inline std::size_t capacity() const noexcept { return capacity_mask_ + 1; }

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
    std::fill_n(storage, size, kEmptyMeta);
    return storage;
  }

  // Returns the byte that stands in for a value with the given hash, which a
  // probe compares before it reaches for the value itself.
  //
  // Taken from the high bits, because the low ones already choose the slot: a
  // byte drawn from those would agree wherever the slot did and so reject
  // nothing, leaving every probe to compare the value. That holds while a
  // table has fewer than 2^25 slots, which is where the bits that choose the
  // slot would start to reach the ones used here.
  static constexpr std::uint8_t hash_meta(std::size_t hash) noexcept {
    return (hash >> 25) & 0b01111111;
  }

  static constexpr std::size_t kGroupSize = sizeof(std::uint64_t);
  static constexpr std::uint8_t kEmptyMeta = 0b10000000;
  static constexpr std::uint8_t kRemovedMeta = 0b11111110;
  static constexpr std::uint64_t kMetaOnes = 0x0101010101010101ULL;
  static constexpr std::uint64_t kMetaHighs = 0x8080808080808080ULL;

  // Returns the meta bytes of the group that starts at `offset`.
  inline std::uint64_t group(std::size_t offset) const noexcept {
    std::uint64_t group_meta;
    std::memcpy(&group_meta, meta(offset), sizeof(group_meta));
    return group_meta;
  }

  // Returns the bytes of `group_meta` that equal `value`, each marked by its
  // high bit. A byte holding a value is under `kEmptyMeta` and a meta byte
  // asked about here never is, so the two cannot be taken for one another.
  static constexpr std::uint64_t match(std::uint64_t group_meta,
                                       std::uint8_t value) noexcept {
    const std::uint64_t diff = group_meta ^ (kMetaOnes * value);
    return (diff - kMetaOnes) & ~diff & kMetaHighs;
  }

  // Returns the offset of the first byte `matches` marks, within the group
  // that starts at `offset`.
  static constexpr std::size_t first_match(std::size_t offset,
                                           std::uint64_t matches) noexcept {
    static_assert(std::endian::native == std::endian::little,
                  "the first byte of a group is the lowest of the word");
    return offset + (std::size_t(std::countr_zero(matches)) / 8);
  }

  static constexpr bool full(std::uint8_t meta) noexcept {
    return (meta & 0b10000000) == 0;
  }

  static constexpr bool empty(std::uint8_t meta) noexcept {
    return meta == kEmptyMeta;
  }

  inline std::size_t next_full(std::size_t pos) const noexcept {
    while (pos < capacity() && !full(*meta(pos))) ++pos;
    return pos;
  }

  void rehash() noexcept {
    const std::size_t new_capacity =
        full_slots_count_ > (capacity() >> 2) ? capacity() << 1 : capacity();
    *this =
        std::move(HashTable(new_capacity).with_content_from(std::move(*this)));
  }

  // Returns an offset that corresponds to the given projection or `capacity()`.
  inline std::size_t find_offset(const P& proj) const {
    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = hash_meta(proj_hash);

    std::size_t offset = group_of(proj_hash);
    for (std::size_t step = 1;; ++step) {
      const std::uint64_t group_meta = group(offset);

      // Every slot of the group whose meta byte stands for this projection,
      // read off the word at once rather than a byte at a time.
      for (std::uint64_t matches = match(group_meta, proj_meta); matches != 0;
           matches &= matches - 1) {
        const std::size_t match_offset = first_match(offset, matches);
        if (Project(*slot(match_offset)) == proj) return match_offset;
      }

      // An empty slot is the end of the chain, so `proj` is not here.
      if (match(group_meta, kEmptyMeta) != 0) return capacity();

      offset = next_group(offset, step);
    }
  }

  // Moves the content of `other` into this table, which must be empty.
  inline HashTable& with_content_from(HashTable&& other) noexcept {
    for (std::size_t pos = 0; pos < other.capacity(); ++pos) {
      const std::uint8_t proj_meta = *other.meta(pos);
      if (!full(proj_meta)) continue;

      V& value = *other.slot(pos);
      const P& proj = Project(value);
      const std::size_t proj_hash = Hash(proj);

      // This table is empty, so the first slot of the chain holding nothing
      // is where the value belongs.
      std::size_t offset = group_of(proj_hash);
      for (std::size_t step = 1;; ++step) {
        if (const std::uint64_t frees = group(offset) & kMetaHighs;
            frees != 0) {
          const std::size_t free_offset = first_match(offset, frees);

          *meta(free_offset) = proj_meta;
          new (slot(free_offset)) V(std::move(value));
          break;
        }

        offset = next_group(offset, step);
      }
    }
    // Only the full slots of `other` were carried over, so this table has no
    // slot that a removal emptied and nothing beyond its values to count
    // against its capacity.
    full_slots_count_ = other.full_slots_count_;
    non_empty_slots_count_ = other.full_slots_count_;
    return *this;
  }

  // Destroys the values in the full slots of this table if needed.
  void destroy_values() noexcept {
    if constexpr (!std::is_trivially_destructible_v<V>) {
      for (std::size_t pos = 0; pos < capacity(); ++pos) {
        if (full(*meta(pos))) slot(pos)->~V();
      }
    }
  }

  // Returns how many slots may be spoken for before the table is rebuilt.
  //
  // Half, which is enough room that a search almost always settles in the
  // first group it reads. Seven eighths was measured against this: it halves
  // what a table takes up and leaves the chains nearly as short, but a fuller
  // group answers a search with more candidates to compare, which costs a
  // lookup more than it saves a rebuild.
  inline std::size_t max_load() const noexcept { return capacity() >> 1; }

  // Returns the group that holds the slot `hash` belongs to.
  inline std::size_t group_of(std::size_t hash) const noexcept {
    return hash & capacity_mask_ & ~(kGroupSize - 1);
  }

  // Returns the group that comes `step` groups after `offset`. A capacity is
  // a power of two and so is the number of groups in it, which is what makes
  // these steps reach every one of them.
  inline std::size_t next_group(std::size_t offset,
                                std::size_t step) const noexcept {
    return (offset + step * kGroupSize) & capacity_mask_;
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
