#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <utility>

namespace lucid {

// Returns a hash of the given argument.
template <typename T>
std::size_t Hash(T);

template <>
std::size_t Hash<int>(int i) {
  i = (i ^ 61) ^ (i >> 16);
  i = i + (i << 3);
  i = i ^ (i >> 4);
  i = i * 0x27d4eb2d;
  i = i ^ (i >> 15);
  return i;
}

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
 public:
  HashMap() : capacity_sub_one_(63), size_(0) {
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, Capacity() + Capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + Capacity(), 0);
  }

  ~HashMap() { std::free(storage_); }

  // Returns the value that corresponds to the given `key` or nullopt.
  inline std::optional<V> Find(K key) const {
    const std::size_t key_hash = Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;
    for (std::size_t pos = key_hash;; ++pos) {
      const std::uint8_t pos_meta = *(meta() + (pos & capacity_sub_one_));
      if (pos_meta == 0) return std::nullopt;

      if (pos_meta == key_meta) {
        const std::pair<K, V>* entry = slots() + (pos & capacity_sub_one_);
        if (entry->first == key) [[likely]] {
          return entry->second;
        }
      }
    }
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V value) {
    if (size_ > (capacity_sub_one_ / 4 * 3)) [[unlikely]] {
      Resize();
    }

    const std::size_t key_hash = Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;

    std::size_t pos = key_hash & capacity_sub_one_;
    while (true) {
      const std::uint8_t pos_meta = *(meta() + pos);
      if (pos_meta == 0) {
        *(meta() + pos) = key_meta;
        *(slots() + pos) = std::make_pair(key, value);
        ++size_;
        return true;
      }
      if (pos_meta == key_meta && (slots() + pos)->first == key) return false;
      pos = (pos + 1) & capacity_sub_one_;
    }
  }

  // Returns the number of key-value pairs inserted so far.
  inline std::size_t Size() const { return size_; }

 private:
  inline std::size_t Capacity() const { return capacity_sub_one_ + 1; }

  inline void Resize() {
    const std::size_t old_capacity = Capacity();
    std::uint8_t* old_storage = storage_;
    const auto* old_slots =
        reinterpret_cast<std::pair<K, V>*>(old_storage + old_capacity);

    capacity_sub_one_ = (old_capacity << 1) - 1;
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

  std::size_t capacity_sub_one_;
  std::size_t size_;
  std::uint8_t* storage_;
};

}  // namespace lucid
