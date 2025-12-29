#pragma once

#include <utility>

#include "lucid/hash_table.h"
#include "lucid/optional_ref.h"

namespace lucid {

// A hash table that maps keys of type `K` to values of type `V`.
template <typename K, typename V>
class HashMap {
 public:
  bool operator==(const HashMap&) const noexcept = default;

  // Returns the value that corresponds to the given `key` if the table contains
  // it. Otherwise returns nullopt.
  inline OptionalRef<V> Find(const K& key) const {
    return table_.Find(key).transform(&std::get<1, K, V>);
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise returns false.
  inline bool Insert(K key, V value) {
    return table_.Insert(std::make_pair(std::move(key), std::move(value)));
  }

  // Inserts the given `key` and `value` pair and returns true if `key` is not
  // already inserted. Otherwise overrides the value and returns false.
  inline bool Set(K key, V value) {
    return table_.Set(std::make_pair(std::move(key), std::move(value)));
  }

  // Removes the given `key` and its corresponding value from the map and
  // returns true if present. Otherwise returns false.
  inline bool Remove(const K& key) { return table_.Remove(key); }

  // Returns the number of unique keys inserted so far.
  inline std::size_t Size() const { return table_.Size(); }

 private:
  HashTable<std::pair<K, V>, K, &std::get<0, K, V>> table_;
};

}  // namespace lucid
