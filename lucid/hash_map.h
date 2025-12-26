#pragma once

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string_view>
#include <utility>

namespace lucid {

// Returns a combination of two given hashes.
template <typename... Hs>
inline std::size_t HashCombine(Hs&&... hs) {
  return (std::rotl(hs, 7) ^ ...);
}

// Returns a hash of the given value.
template <typename T>
struct Hasher;

template <>
struct Hasher<std::uint32_t> {
  static std::size_t Hash(std::uint32_t v) {
    // Low bias 32-bit hash function discovered by
    // https://github.com/skeeto/hash-prospector.
    v ^= v >> 16;
    v *= 0x7feb352d;
    v ^= v >> 15;
    v *= 0x846ca68b;
    v ^= v >> 16;
    return v;
  }
};

template <>
struct Hasher<std::int32_t> {
  static std::size_t Hash(std::uint32_t v) {
    return Hasher<std::uint32_t>::Hash(v);
  }
};

template <>
struct Hasher<std::uint8_t> {
  static std::size_t Hash(std::uint32_t v) {
    return Hasher<std::uint32_t>::Hash(v);
  }
};

template <>
struct Hasher<std::uint16_t> {
  static std::size_t Hash(std::uint32_t v) {
    return Hasher<std::uint32_t>::Hash(v);
  }
};

template <>
struct Hasher<std::string_view> {
  static std::size_t Hash(std::string_view v) {
    std::size_t h = Hasher<std::uint32_t>::Hash(0);
    const std::size_t words_end = (v.size() >> 2) << 2;
    std::size_t i = 0;
    for (; i < words_end; i += 4) {
      const std::uint32_t word =
          (v[i] << 16) | (v[i + 1] << 8) | (v[i + 2] << 4) | v[i + 3];
      h = HashCombine(h, Hasher<std::uint32_t>::Hash(word));
    }
    for (; i < v.size(); ++i) {
      h = HashCombine(h, Hasher<std::uint8_t>::Hash(v[i]));
    }
    return h;
  }
};

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
 public:
  HashMap() : capacity_mask_(initial_capacity() - 1), size_(0) {
    storage_ = static_cast<std::uint8_t*>(
        std::aligned_alloc(alignof(std::max_align_t),
                           capacity() + capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + capacity(), 0);
  }

  HashMap(const HashMap& other) {
    capacity_mask_ = other.capacity_mask_;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, capacity() + capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + capacity(), 0);

    for (std::size_t pos = 0; pos < other.capacity(); ++pos) {
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
    const std::size_t key_hash = Hasher<K>::Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;
    std::size_t offset = key_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == key_meta) {
        const std::pair<K, V>* entry = slots() + offset;
        if (entry->first == key) return entry->second;
      }
      if (offset_meta == 0) return std::nullopt;
    }
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V value) {
    if (size_ > (capacity_mask_ >> 1)) Resize();

    const std::size_t key_hash = Hasher<K>::Hash(key);
    const std::uint8_t key_meta = key_hash | 0b10000000;
    std::size_t offset = key_hash;
    for (std::size_t i = 0;; ++i) {
      offset = (offset + i) & capacity_mask_;

      const std::uint8_t offset_meta = *(meta() + offset);
      if (offset_meta == 0) {
        *(meta() + offset) = key_meta;
        *(slots() + offset) = std::make_pair(key, value);
        ++size_;
        return true;
      }
      if ((slots() + offset)->first == key) return false;
    }
  }

  // Returns the number of key-value pairs inserted so far.
  inline std::size_t Size() const { return size_; }

 private:
  static constexpr std::size_t initial_capacity() {
    static constexpr std::size_t max_align = alignof(std::max_align_t);
    return ((64 + max_align - 1) / max_align) * max_align;
  }

  inline std::size_t capacity() const { return capacity_mask_ + 1; }

  inline void Resize() {
    const std::size_t old_capacity = capacity();
    std::uint8_t* old_meta = meta();
    const std::pair<K, V>* old_slots = slots();

    capacity_mask_ = (old_capacity << 1) - 1;
    size_ = 0;
    storage_ = static_cast<std::uint8_t*>(std::aligned_alloc(
        64, capacity() + capacity() * sizeof(std::pair<K, V>)));
    std::fill(meta(), meta() + capacity(), 0);

    for (std::size_t pos = 0; pos < old_capacity; ++pos) {
      if (*(old_meta + pos) > 0) {
        Insert((old_slots + pos)->first, (old_slots + pos)->second);
      }
    }

    std::free(old_meta);
  }

  inline std::uint8_t* meta() const { return storage_; }

  inline std::pair<K, V>* slots() const {
    return reinterpret_cast<std::pair<K, V>*>(storage_ + capacity());
  }

  std::size_t capacity_mask_;
  std::size_t size_;
  std::uint8_t* storage_ = nullptr;
};

}  // namespace lucid
