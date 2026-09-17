#pragma once

#include <optional>
#include <utility>

#include "lucid/core/container/hash_table.h"

namespace lucid {

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
  using HashTableT = HashTable<std::pair<K, V>, K, &std::get<0, K, V>>;

 public:
  using value_type = std::pair<K, V>;

  bool operator==(const HashMap&) const noexcept = default;

  // Returns a reference to the value that corresponds to the given `key` if the
  // table contains it. Otherwise returns nullopt.
  inline std::optional<V&> Get(const K& key) {
    if (auto it = table_.Find(key); it != end()) return it->second;
    return std::nullopt;
  }

  // Returns a const reference to the value that corresponds to the given `key`
  // if the table contains it. Otherwise returns nullopt.
  inline std::optional<const V&> Get(const K& key) const {
    if (auto it = table_.Find(key); it != end()) return it->second;
    return std::nullopt;
  }

  // Finds the value that corresponds to the given `key` if the table contains
  // it or inserts one that's constructed with the remaining arguments.
  template <typename... Args>
  inline V& Emplace(const K& key, Args&&... args) {
    auto [val_slot, allocated] = table_.FindOrAlloc(key);
    if (allocated) {
      new (val_slot) std::pair<K, V>(key, V(std::forward<Args>(args)...));
    }
    return val_slot->second;
  }

  // Inserts the given `key` and `val` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V val) {
    return table_.Insert(std::make_pair(std::move(key), std::move(val)));
  }

  // Inserts the given `key` and `val` pair and returns true if `key` is not
  // already inserted. Otherwise overrides the value and returns false.
  inline bool Set(K key, V val) {
    return table_.Set(std::make_pair(std::move(key), std::move(val)));
  }

  // Removes the given `key` and its corresponding value from the map and
  // returns the value if present. Otherwise returns nullopt.
  inline std::optional<V> Remove(const K& key) {
    return table_.Remove(key).transform(
        [](auto&& r) { return std::get<1>(std::forward<decltype(r)>(r)); });
  }

  // Returns the number of unique keys inserted so far.
  inline std::size_t size() const { return table_.size(); }

  // Returns true iff there are no key-value pairs in the map.
  inline bool empty() const { return size() == 0; }

  // Returns an iterator referring to the first key-value pair in the map or
  // `end()`, if there isn't one.
  inline HashTableT::Iterator begin() { return table_.begin(); }

  // Returns a const iterator referring to the first key-value pair in the map
  // or `end()`, if there isn't one.
  inline HashTableT::ConstIterator begin() const { return table_.begin(); }

  // Returns an iterator past the last key-value pair in the map.
  inline HashTableT::Iterator end() { return table_.end(); }

  // Returns a const iterator past the last key-value pair in the map.
  inline HashTableT::ConstIterator end() const { return table_.end(); }

 private:
  HashTableT table_;
};

}  // namespace lucid
