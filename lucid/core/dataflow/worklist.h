#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace lucid {

// Models a finite set of elements.
template <typename D, typename E>
concept FiniteDomain = requires(D d, E e) {
  std::same_as<typename D::element_type, E>;
  { d.size() } -> std::same_as<std::size_t>;
  { d.id(e) } -> std::same_as<std::size_t>;
};

// A worklist of the elements of a finite domain, taken out in an order fixed
// when it is made: of the elements in the worklist, the one that comes first in
// that order comes out first. An element can appear at most once in the
// worklist.
//
// It is held as a bit for each place in the order, so that putting an element
// in or taking one out sets or clears a bit, where a heap would have to be put
// back in order each time.
template <typename T, FiniteDomain<T> D>
class Worklist {
 public:
  // `order` must hold every element of `domain` once.
  Worklist(D domain, std::vector<T> order)
      : domain_(std::move(domain)),
        order_(std::move(order)),
        places_(domain_.size()),
        words_((order_.size() + kWordBits - 1) / kWordBits, 0) {
    assert(order_.size() == domain_.size());
    for (std::size_t place = 0; place < order_.size(); ++place) {
      places_[domain_.id(order_[place])] = place;
    }
  }

  // Returns whether the worklist is empty.
  bool empty() const { return count_ == 0; }

  // Removes and returns the element that comes first in the order.
  T pop() {
    assert(!empty());
    while (words_[first_word_] == 0) ++first_word_;

    std::uint64_t& word = words_[first_word_];
    const std::size_t place = first_word_ * kWordBits + std::countr_zero(word);
    word &= word - 1;
    // An empty worklist has no first word, and the next push says where it
    // is, rather than leaving the next pop to look for it from here.
    if (--count_ == 0) first_word_ = words_.size();
    return order_[place];
  }

  // Inserts the element in the worklist if it's not already present.
  void push(T t) {
    const std::size_t place = places_[domain_.id(t)];
    const std::size_t word_index = place / kWordBits;
    const std::uint64_t bit = std::uint64_t{1} << (place % kWordBits);
    if ((words_[word_index] & bit) != 0) return;

    words_[word_index] |= bit;
    ++count_;
    first_word_ = std::min(first_word_, word_index);
  }

  // Inserts the elements from the range that are not already present in the
  // worklist.
  template <typename R>
  void push_range(R&& rg) {
    for (const auto& t : rg) push(t);
  }

  // Inserts every element of the domain.
  void push_all() {
    std::fill(words_.begin(), words_.end(), ~std::uint64_t{0});
    // The places past the last element are never filled, so that taking out
    // the last element leaves the worklist empty.
    if (const std::size_t rest = order_.size() % kWordBits; rest != 0) {
      words_.back() = (std::uint64_t{1} << rest) - 1;
    }
    count_ = order_.size();
    first_word_ = 0;
  }

 private:
  static constexpr std::size_t kWordBits = 64;

  D domain_;
  // The elements, by their place in the order.
  std::vector<T> order_;
  // The place of each element in the order, by its ID in the domain.
  std::vector<std::size_t> places_;
  // A bit for each place in the order, set for the elements in the worklist.
  std::vector<std::uint64_t> words_;
  // No word before this one has a bit set. Past the last word while the
  // worklist is empty.
  std::size_t first_word_ = words_.size();
  // How many elements are in the worklist.
  std::size_t count_ = 0;
};

// Deduction guide to simplify the construction of a worklist.
//
// Example:
//
//   Worklist worklist(domain, order);
template <typename D>
  requires FiniteDomain<D, typename D::element_type>
Worklist(D domain, std::vector<typename D::element_type> order)
    -> Worklist<typename D::element_type, D>;

}  // namespace lucid
