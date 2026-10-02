#pragma once

#include <concepts>
#include <cstddef>
#include <queue>
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

// Models an ordering relation between elements.
template <typename O, typename E>
concept OrderedBefore = requires(O o, E e1, E e2) {
  { o(e1, e2) } -> std::same_as<bool>;
};

// A worklist of elements of type `T`, taken out in the order the comparator of
// type `O` puts them in: an element it puts before another comes out first. An
// element can appear at most once in the worklist.
template <typename T, FiniteDomain<T> D, OrderedBefore<T> O>
class Worklist {
 public:
  Worklist(D domain, O compare)
      : domain_(std::move(domain)),
        queue_(ComesLater{.order = std::move(compare)}),
        present_(domain_.size(), false) {}

  // Returns whether the worklist is empty.
  bool empty() const { return queue_.empty(); }

  // Removes and returns the element that comes first in the order.
  T pop() {
    T top = queue_.top();
    queue_.pop();
    present_[domain_.id(top)] = false;
    return top;
  }

  // Inserts and sorts the element in the worklist if it's not already present.
  void push(T t) {
    if (present_[domain_.id(t)]) return;
    present_[domain_.id(t)] = true;
    queue_.push(t);
  }

  // Inserts and sorts the elements from the range that are not already present
  // in the worklist.
  template <typename R>
  void push_range(R&& rg) {
    for (const auto& t : rg) push(t);
  }

 private:
  // The order turned round. A priority queue takes out what compares greatest,
  // so it is handed this, to take out what the order puts first.
  struct ComesLater {
    O order;

    bool operator()(const T& lhs, const T& rhs) const {
      return order(rhs, lhs);
    }
  };

  D domain_;
  std::priority_queue<T, std::vector<T>, ComesLater> queue_;
  std::vector<bool> present_;
};

// Deduction guide to simplify the construction of a worklist.
//
// Example:
//
//   Worklist worklist(domain, compare);
template <typename D, OrderedBefore<typename D::element_type> O>
  requires FiniteDomain<D, typename D::element_type>
Worklist(D domain, O ordered_before)
    -> Worklist<typename D::element_type, D, O>;

}  // namespace lucid
