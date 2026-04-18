#pragma once

#include <optional>
#include <utility>

#include "lucid/core/container/hash_table.h"

namespace lucid {

// A hash table that contains values of type `V`.
template <typename V>
class HashSet {
  using HashTableT = HashTable<V>;

 public:
  using value_type = V;

  bool operator==(const HashSet&) const noexcept = default;

  // Returns true iff the table contains `val`.
  inline bool Contains(V val) const { return table_.Find(val).has_value(); }

  // Inserts the given `val` and returns true if it is not already in the
  // table. Otherwise returns false.
  inline bool Insert(V val) { return table_.Insert(std::move(val)); }

  // Removes the given `val` from the set and returns it if present.
  // Otherwise returns nullopt.
  inline std::optional<V> Remove(const V& val) { return table_.Remove(val); }

  // Returns the number of unique values inserted so far.
  inline std::size_t size() const { return table_.size(); }

  // Returns true iff there are no values in the set.
  inline bool empty() const { return size() == 0; }

  // Returns a const iterator referring to the first value in the set or
  // `end()`, if there isn't one.
  inline HashTableT::ConstIterator begin() const { return table_.begin(); }

  // Returns a const iterator past the last value in the set.
  inline HashTableT::ConstIterator end() const { return table_.end(); }

 private:
  HashTableT table_;
};

}  // namespace lucid
