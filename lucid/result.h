#pragma once

#include <ostream>
#include <utility>
#include <variant>

namespace lucid {

// A result that is either a value of type `V` or an error of one of the types
// `Es`.
template <typename V, typename... Es>
class Result {
 public:
  Result(V&& v) : state_(std::move(v)) {}

  template <typename E>
  Result(const E& e) : state_(e) {}

  template <typename... Ts>
  Result(const Result<V, Ts...>& r)
      : state_(std::visit([](auto&& a) -> std::variant<V, Es...> { return a; },
                          r.state_)) {}

  template <typename... Ts>
  Result(Result<V, Ts...>&& r)
      : state_(std::visit([](auto&& a) -> std::variant<V, Es...> { return a; },
                          std::move(r.state_))) {}

  Result& operator=(const Result&) = delete;
  Result& operator=(Result&&) = delete;

  // Returns true iff the result contains a value.
  bool HasValue() const { return std::holds_alternative<V>(state_); }

  // Returns true iff the result contains an error.
  bool HasError() const { return !HasValue(); }

  // Returns true iff the result contains an error of type `E`.
  template <typename E>
  bool HasError() const {
    const E* e = std::get_if<E>(&state_);
    return e != nullptr;
  }

  // Returns a reference to the value that the result contains.
  //
  // Requirements:
  // - Must be called only if the result contains a value.
  const V& GetValue() const { return std::get<V>(state_); }

  // Outputs an error to `out`.
  //
  // Requirements:
  // - Must be called only if the result contains an error.
  void OutputError(std::ostream& out) const {
    std::visit([&out](auto&& a) { out << a << "\n"; }, state_);
  }

 private:
  template <typename, typename...>
  friend class Result;

  std::variant<V, Es...> state_;
};

}  // namespace lucid
