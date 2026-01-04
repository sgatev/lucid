#pragma once

#include <utility>

#include "lucid/hash_table.h"

namespace lucid {

// A hash table that contains values of type `V`.
template <typename V>
class HashSet {
  using HashTableT = HashTable<V>;

 public:
  using value_type = V;

  bool operator==(const HashSet&) const noexcept = default;

  // Returns true iff the table contains `value`.
  inline bool Contains(V value) const { return table_.Find(value).has_value(); }

  // Inserts the given `value` and returns true if it is not already in the
  // table. Otherwise returns false.
  inline bool Insert(V value) { return table_.Insert(std::move(value)); }

  // Removes the given `value` from the set and returns true if present.
  // Otherwise returns false.
  inline bool Remove(const V& value) { return table_.Remove(value); }

  // Returns the number of unique values inserted so far.
  inline std::size_t Size() const { return table_.Size(); }

  // Returns a const iterator referring to the first value in the set or
  // `end()`, if there isn't one.
  inline HashTableT::ConstIterator begin() const { return table_.begin(); }

  // Returns a const iterator past the last value in the set.
  inline HashTableT::ConstIterator end() const { return table_.end(); }

 private:
  HashTableT table_;
};

}  // namespace lucid
