#pragma once

#include <queue>
#include <ranges>
#include <unordered_set>
#include <vector>

namespace lucid {

// A worklist of elements of type `T` ordered using comparator of type `C`. An
// element can appear at most once in the worklist.
template <typename T, typename C>
class Worklist {
 public:
  explicit Worklist(C comp) : queue_(std::move(comp)) {}

  // Returns whether the worklist is empty.
  bool empty() const { return queue_.empty(); }

  // Removes and returns the top element in the worklist.
  T pop() {
    T top = queue_.top();
    queue_.pop();
    inserted_.erase(top);
    return top;
  }

  // Inserts and sorts the element in the worklist if it's not already present.
  void push(T t) {
    if (inserted_.insert(t).second) queue_.push(t);
  }

  // Inserts and sorts the elements from the range that are not already present
  // in the worklist.
  template <typename R>
  void push_range(R&& rg) {
    queue_.push_range(rg | std::views::filter([&](const T& t) {
                        return !inserted_.contains(t);
                      }));
    inserted_.insert_range(rg);
  }

 private:
  std::priority_queue<T, std::vector<T>, C> queue_;
  std::unordered_set<T> inserted_;
};

}  // namespace lucid
