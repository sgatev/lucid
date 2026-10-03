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

  // A table with no slots, which takes them when it is first given something
  // to hold. Nothing is allocated for one that never is, so an empty table
  // costs no more than the members below.
  HashTable() = default;

  explicit HashTable(std::size_t wanted_capacity)
      : capacity_(std::bit_ceil(wanted_capacity)),
        storage_(alloc_storage(capacity_)) {}

  // Leaves `other` the table with no slots, rather than one that says it has
  // slots it no longer holds the storage for and would be walked past the end
  // of. That is a state a table can be left in only because an empty one
  // needs no storage.
  HashTable(HashTable&& other) noexcept
      : capacity_(std::exchange(other.capacity_, 0)),
        full_slots_count_(std::exchange(other.full_slots_count_, 0)),
        non_empty_slots_count_(std::exchange(other.non_empty_slots_count_, 0)),
        storage_(std::exchange(other.storage_, nullptr)) {}

  HashTable(const HashTable& other)
      : capacity_(other.capacity_),
        full_slots_count_(other.full_slots_count_),
        non_empty_slots_count_(other.non_empty_slots_count_),
        storage_(other.storage_ == nullptr ? nullptr
                                           : alloc_storage(capacity_)) {
    if (storage_ == nullptr) return;

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
    std::swap(capacity_, other.capacity_);
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
    // A table with no slots has nowhere to put a value, so the first thing it
    // is given is what makes it take them.
    if (storage_ == nullptr) {
      capacity_ = kInitialCapacity;
      storage_ = alloc_storage(capacity_);
    }

    const std::size_t proj_hash = Hash(proj);
    const std::uint8_t proj_meta = hash_meta(proj_hash);

    const Probe probed = probe(proj, proj_hash, proj_meta);
    if (probed.held) return std::make_pair(slot(probed.offset), false);

    // The table is rebuilt only for a value that is going in, and so only
    // once the search has not found it. Rebuilding moves every value, and
    // with them the order the table is walked in, so asking for a value it
    // already holds has to leave it as it was. The slot the search found goes
    // with the rebuild, and the value is known not to be held, so all that is
    // left to find is a free slot.
    std::size_t offset = probed.offset;
    if (non_empty_slots_count_ > max_load()) {
      rehash();
      offset = free_offset(proj_hash);
    }
    return std::make_pair(alloc_at(offset, proj_meta), true);
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
  inline std::size_t capacity() const noexcept { return capacity_; }

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
        full_slots_count_ > (capacity_ >> 2) ? capacity_ << 1 : capacity_;
    *this =
        std::move(HashTable(new_capacity).with_content_from(std::move(*this)));
  }

  // Returns an offset that corresponds to the given projection or `capacity()`.
  // Where a search for a value lands: the slot that holds it, or, when none
  // does, the first slot of its chain holding nothing, which is where it goes.
  struct Probe {
    std::size_t offset;
    bool held;
  };

  // Searches the chain of `proj`, whose hash and meta byte are given.
  inline Probe probe(const P& proj, std::size_t proj_hash,
                     std::uint8_t proj_meta) const {
    std::size_t first_free_offset = capacity();
    std::size_t offset = group_of(proj_hash);
    for (std::size_t step = 1;; ++step) {
      const std::uint64_t group_meta = group(offset);

      for (std::uint64_t matches = match(group_meta, proj_meta); matches != 0;
           matches &= matches - 1) {
        const std::size_t match_offset = first_match(offset, matches);
        if (Project(*slot(match_offset)) == proj) {
          return {.offset = match_offset, .held = true};
        }
      }

      // A slot holding nothing is one that carries a high bit, whether it was
      // never filled or a removal emptied it.
      if (first_free_offset == capacity()) {
        if (const std::uint64_t frees = group_meta & kMetaHighs; frees != 0) {
          first_free_offset = first_match(offset, frees);
        }
      }

      // An empty slot is the end of the chain, so `proj` is not here.
      if (match(group_meta, kEmptyMeta) != 0) {
        return {.offset = first_free_offset, .held = false};
      }

      offset = next_group(offset, step);
    }
  }

  // Returns the first slot holding nothing along the chain of `hash`, for a
  // value known not to be held, which needs no comparing on the way.
  inline std::size_t free_offset(std::size_t hash) const {
    std::size_t offset = group_of(hash);
    for (std::size_t step = 1;; ++step) {
      if (const std::uint64_t frees = group(offset) & kMetaHighs; frees != 0) {
        return first_match(offset, frees);
      }
      offset = next_group(offset, step);
    }
  }

  // Counts the slot at `offset` as full and marks it with `proj_meta`, and
  // returns it for the value to be put in.
  inline V* alloc_at(std::size_t offset, std::uint8_t proj_meta) {
    ++full_slots_count_;
    if (empty(*meta(offset))) ++non_empty_slots_count_;
    *meta(offset) = proj_meta;
    return slot(offset);
  }

  inline std::size_t find_offset(const P& proj) const {
    // A table with no slots holds nothing, and has no group to read.
    if (storage_ == nullptr) return capacity();

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
    return hash & capacity_mask() & ~(kGroupSize - 1);
  }

  // Returns the group that comes `step` groups after `offset`. A capacity is
  // a power of two and so is the number of groups in it, which is what makes
  // these steps reach every one of them.
  inline std::size_t next_group(std::size_t offset,
                                std::size_t step) const noexcept {
    return (offset + step * kGroupSize) & capacity_mask();
  }

  // Returns the low bits of an offset that a capacity spans, which is what
  // takes a hash to a slot. A capacity is a power of two, so these are every
  // bit under it.
  inline std::size_t capacity_mask() const noexcept { return capacity_ - 1; }

  inline std::uint8_t* meta(std::size_t i) const noexcept {
    return storage_ + i;
  }

  inline V* slot(std::size_t i) const noexcept {
    return reinterpret_cast<V*>(storage_ + capacity()) + i;
  }

  std::size_t capacity_ = 0;
  std::size_t full_slots_count_ = 0;
  std::size_t non_empty_slots_count_ = 0;
  std::uint8_t* storage_ = nullptr;
};

}  // namespace lucid
