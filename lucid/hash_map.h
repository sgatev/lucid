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
std::size_t Hash(const T&);

template <>
std::size_t Hash<int>(const int& i) {
  return i;
}

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
 public:
  HashMap() : capacity_(16), size_(0) {
    storage_ = static_cast<std::uint8_t*>(
        std::malloc(capacity_ + capacity_ * sizeof(std::pair<K, V>)));
    std::fill(Metadata(), Metadata() + capacity_, 0);
  }

  ~HashMap() { std::free(storage_); }

  // Returns the value that corresponds to the given `key` or nullopt.
  inline std::optional<V> Find(K key) const {
    std::size_t key_hash = Hash(key);
    std::uint8_t key_meta = 0b10000000 | (key_hash & 0b01111111);

    std::size_t pos = key_hash % capacity_;
    while (true) {
      std::uint8_t pos_meta = *(Metadata() + pos);
      if (pos_meta == 0) break;

      if (pos_meta == key_meta) {
        const std::pair<K, V>* entry = Slots() + pos;
        if (entry->first == key) return entry->second;
      }

      pos = (pos + 1) % capacity_;
    }

    return std::nullopt;
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V value) {
    if ((size_ << 1) > capacity_) Resize();

    std::size_t key_hash = Hash(key);
    std::uint8_t key_meta = 0b10000000 | (key_hash & 0b01111111);

    std::size_t pos = key_hash % capacity_;
    while (true) {
      std::uint8_t pos_meta = *(Metadata() + pos);
      if (pos_meta == 0) break;
      if (pos_meta == key_meta && (Slots() + pos)->first == key) return false;
      pos = (pos + 1) % capacity_;
    }

    *(Metadata() + pos) = key_meta;
    *(Slots() + pos) = {key, value};
    ++size_;
    return true;
  }

  // Returns the number of key-value pairs inserted so far.
  inline std::size_t Size() const { return size_; }

 private:
  inline void Resize() {
    std::size_t old_capacity = capacity_;
    std::uint8_t* old_storage = storage_;
    std::pair<K, V>* old_slots =
        reinterpret_cast<std::pair<K, V>*>(old_storage + old_capacity);

    capacity_ <<= 1;
    storage_ = static_cast<std::uint8_t*>(
        std::malloc(capacity_ + capacity_ * sizeof(std::pair<K, V>)));
    std::fill(Metadata(), Metadata() + capacity_, 0);
    size_ = 0;

    for (std::size_t pos = 0; pos < old_capacity; ++pos) {
      if (*(old_storage + pos) > 0) {
        Insert((old_slots + pos)->first, (old_slots + pos)->second);
      }
    }

    std::free(old_storage);
  }

  inline std::uint8_t* Metadata() const { return storage_; }

  inline std::pair<K, V>* Slots() const {
    return reinterpret_cast<std::pair<K, V>*>(storage_ + capacity_);
  }

  std::size_t capacity_;
  std::size_t size_;
  std::uint8_t* storage_;
};

}  // namespace lucid
