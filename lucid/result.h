#pragma once

#include <ostream>
#include <type_traits>
#include <utility>
#include <variant>

namespace lucid {

// A result that is an error of one of the types `Es`.
template <typename... Es>
class ErrorResult {
 public:
  // Returns true iff the result contains an error of type `E`.
  template <typename E>
  bool Is() const {
    return std::holds_alternative<E>(state_);
  }

  template <typename E>
  bool operator==(const E& e) const {
    return Is<E>() && std::get<E>(state_) == e;
  }

 private:
  template <typename V, typename... Ts>
  friend class Result;

  ErrorResult(std::variant<Es...> state) : state_(std::move(state)) {}

  std::variant<Es...> state_;
};

// A result that is either a value of type `V` or an error of one of the types
// `Es`.
template <typename V, typename... Es>
class Result {
 public:
  using Errors = std::variant<Es...>;

  Result(V&& v) : state_(std::move(v)) {}

  template <typename E>
    requires(std::is_same_v<E, Es> || ...)
  Result(const E& e) : state_(Errors(e)) {}

  template <typename... Ts>
  Result(const Result<V, Ts...>& r)
      : state_(std::visit(
            [](auto&& a) -> std::variant<V, Errors> {
              using T = std::decay_t<decltype(a)>;
              if constexpr (std::is_same_v<T, V>) {
                return a;
              } else {
                return std::visit([](auto&& a) -> Errors { return a; }, a);
              }
            },
            r.state_)) {}

  template <typename... Ts>
  Result(ErrorResult<Ts...> e)
      : state_(std::visit([](auto&& a) -> Errors { return a; },
                          std::move(e.state_))) {}

  Result& operator=(const Result&) = delete;
  Result& operator=(Result&&) = delete;

  // Returns true iff the result contains a value.
  bool HasValue() const { return std::holds_alternative<V>(state_); }

  // Returns true iff the result contains an error.
  bool HasError() const { return !HasValue(); }

  // Returns true iff the result contains an error of type `E`.
  template <typename E>
  bool HasError() const {
    const E* e = std::get_if<E>(&std::get<Errors>(state_));
    return e != nullptr;
  }

  // Returns a reference to the value that the result contains.
  //
  // Requires:
  // - Must be called only if the result contains a value.
  const V& GetValue() const { return std::get<V>(state_); }
  V& GetValue() { return std::get<V>(state_); }

  // Returns the error that the result contains.
  //
  // Requires:
  // - Must be called only if the result contains an error.
  ErrorResult<Es...> GetError() const {
    return ErrorResult<Es...>(std::get<Errors>(state_));
  }

  // Outputs an error to `out`.
  //
  // Requires:
  // - Must be called only if the result contains an error.
  void OutputError(std::ostream& out) const {
    std::visit([&out](auto&& a) { out << a << "\n"; },
               std::get<Errors>(state_));
  }

 private:
  template <typename, typename...>
  friend class Result;

  std::variant<V, Errors> state_;
};

// A result that is either void or an error of one of the types `Es`.
template <typename... Es>
class Result<void, Es...> {
 public:
  using Errors = std::variant<Es...>;

  Result() : state_(std::monostate()) {}

  template <typename E>
    requires(std::is_same_v<E, Es> || ...)
  Result(const E& e) : state_(Errors(e)) {}

  template <typename... Ts>
  Result(const Result<void, Ts...>& r)
      : state_(std::visit(
            [](auto&& a) -> std::variant<std::monostate, Errors> {
              using T = std::decay_t<decltype(a)>;
              if constexpr (std::is_same_v<T, std::monostate>) {
                return std::monostate();
              } else {
                return std::visit([](auto&& a) -> Errors { return a; }, a);
              }
            },
            r.state_)) {}

  template <typename... Ts>
  Result(ErrorResult<Ts...> e)
      : state_(std::visit([](auto&& a) -> Errors { return a; },
                          std::move(e.state_))) {}

  Result& operator=(const Result&) = delete;
  Result& operator=(Result&&) = delete;

  // Returns true iff the result contains a value.
  bool HasValue() const {
    return std::holds_alternative<std::monostate>(state_);
  }

  // Returns true iff the result contains an error.
  bool HasError() const { return !HasValue(); }

  // Returns true iff the result contains an error of type `E`.
  template <typename E>
  bool HasError() const {
    const E* e = std::get_if<E>(&std::get<Errors>(state_));
    return e != nullptr;
  }

  // Returns the error that the result contains.
  //
  // Requires:
  // - Must be called only if the result contains an error.
  ErrorResult<Es...> GetError() const {
    return ErrorResult<Es...>(std::get<Errors>(state_));
  }

  // Outputs an error to `out`.
  //
  // Requires:
  // - Must be called only if the result contains an error.
  void OutputError(std::ostream& out) const {
    std::visit(
        [&out](auto&& a) {
          using T = std::decay_t<decltype(a)>;
          if constexpr (!std::is_same_v<T, std::monostate>) {
            out << a << "\n";
          }
        },
        std::get<Errors>(state_));
  }

 private:
  template <typename, typename...>
  friend class Result;

  std::variant<std::monostate, Errors> state_;
};

}  // namespace lucid
