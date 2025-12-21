#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <optional>
#include <string_view>
#include <utility>

namespace lucid {

// Returns a hash of the given value.
template <typename T>
std::size_t Hash(T);

template <>
inline std::size_t Hash<std::uint32_t>(std::uint32_t v) {
  v ^= v >> 16;
  v *= 0x21f0aaadU;
  v ^= v >> 15;
  v *= 0x735a2d97U;
  v ^= v >> 15;
  return v;
}

template <>
inline std::size_t Hash<std::int32_t>(std::int32_t v) {
  v ^= v >> 16;
  v *= 0x21f0aaadU;
  v ^= v >> 15;
  v *= 0x735a2d97U;
  v ^= v >> 15;
  return v;
}

template <>
inline std::size_t Hash<std::string_view>(std::string_view v) {
  // TODO: Find a better hash function.
  return std::reduce(v.begin(), v.end(), 0,
                     [](std::size_t h, char c) { return h = (h << 1) + c; });
}

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
 public:
  HashMap() : capacity_mask_(63), size_(0) {
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, Capacity() + Capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + Capacity(), 0);
  }

  HashMap(const HashMap& other) {
    capacity_mask_ = other.capacity_mask_;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, Capacity() + Capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + Capacity(), 0);

    for (std::size_t pos = 0; pos < other.Capacity(); ++pos) {
      if (*(other.meta() + pos) > 0) {
        Insert((other.slots() + pos)->first, (other.slots() + pos)->second);
      }
    }
  }

  ~HashMap() {
    if (storage_ != nullptr) std::free(storage_);
  }

  // Returns the value that corresponds to the given `key` or nullopt.
  inline std::optional<V> Find(K key) const {
    const std::size_t key_hash = Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;
    for (std::size_t i = 0;; ++i) {
      const std::size_t pos = probe(key_hash, i);

      const std::uint8_t pos_meta = *(meta() + pos);
      if (pos_meta == 0) return std::nullopt;

      if (pos_meta == key_meta) {
        const std::pair<K, V>* entry = slots() + pos;
        if (entry->first == key) [[likely]] {
          return entry->second;
        }
      }
    }
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V value) {
    if (size_ > (capacity_mask_ / 4 * 3)) [[unlikely]] {
      Resize();
    }

    const std::size_t key_hash = Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;

    for (std::size_t i = 0;; ++i) {
      const std::size_t pos = probe(key_hash, i);

      const std::uint8_t pos_meta = *(meta() + pos);
      if (pos_meta == 0) {
        *(meta() + pos) = key_meta;
        *(slots() + pos) = std::make_pair(key, value);
        ++size_;
        return true;
      }
      if (pos_meta == key_meta && (slots() + pos)->first == key) return false;
    }
  }

  // Returns the number of key-value pairs inserted so far.
  inline std::size_t Size() const { return size_; }

 private:
  inline std::size_t Capacity() const { return capacity_mask_ + 1; }

  inline void Resize() {
    const std::size_t old_capacity = Capacity();
    std::uint8_t* old_storage = storage_;
    const auto* old_slots =
        reinterpret_cast<std::pair<K, V>*>(old_storage + old_capacity);

    capacity_mask_ = (old_capacity << 1) - 1;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, Capacity() + Capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + Capacity(), 0);

    for (std::size_t pos = 0; pos < old_capacity; ++pos) {
      if (*(old_storage + pos) > 0) {
        Insert((old_slots + pos)->first, (old_slots + pos)->second);
      }
    }

    std::free(old_storage);
  }

  inline std::uint8_t* meta() const { return storage_; }

  inline std::pair<K, V>* slots() const {
    return reinterpret_cast<std::pair<K, V>*>(storage_ + Capacity());
  }

  inline std::size_t probe(std::size_t hash, std::size_t i) const {
    return (hash + (i >> 1) + ((i * i) >> 1)) & capacity_mask_;
  }

  std::size_t capacity_mask_;
  std::size_t size_;
  std::uint8_t* storage_ = nullptr;
};

}  // namespace lucid
