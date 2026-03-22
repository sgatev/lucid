#pragma once

#include <cstdint>
#include <iterator>

namespace lucid {

// A list of successive elements of type `T`.
template <typename T>
class SuccessiveList {
 public:
  using value_type = T;

  class iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = int;

    iterator() : expr_(0) {}

    explicit iterator(T expr) : expr_(expr) {}

    iterator(const iterator& other) = default;
    iterator(iterator&& other) = default;
    iterator& operator=(const iterator& other) = default;
    iterator& operator=(iterator&& other) = default;

    iterator& operator++() {
      ++expr_;
      return *this;
    }

    iterator operator++(int) {
      iterator prev = *this;
      ++*this;
      return prev;
    }

    iterator& operator--() {
      --expr_;
      return *this;
    }

    iterator operator--(int) {
      iterator prev = *this;
      --*this;
      return prev;
    }

    bool operator==(const iterator& other) const {
      return expr_ == other.expr_;
    }

    bool operator!=(const iterator& other) const { return !(*this == other); }

    value_type operator*() const { return expr_; }

   private:
    T expr_;
  };

  SuccessiveList() : size_(0), first_(0) {}

  SuccessiveList(const SuccessiveList& other) = default;
  SuccessiveList(SuccessiveList&& other) = default;

  SuccessiveList& operator=(const SuccessiveList& other) = default;
  SuccessiveList& operator=(SuccessiveList&& other) = default;

  SuccessiveList(std::uint32_t size, T first) : size_(size), first_(first) {}

  // Returns the value at the given index.
  T operator[](std::uint32_t i) const { return first_ + i; }

  // Returns the size of the list.
  std::uint32_t size() const { return size_; }

  // Returns true if and only if the list is empty.
  bool empty() const { return size() == 0; }

  // Returns an iterator to the first value in the list.
  auto begin() const { return iterator(first_); }

  // Returns an iterator following the last value in the list.
  auto end() const { return iterator(first_ + size_); }

 private:
  std::uint32_t size_;
  T first_;
};

}  // namespace lucid
