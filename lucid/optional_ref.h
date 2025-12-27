#pragma once

#include <optional>

namespace lucid {

// Template for classes that may contain a reference to an object of type `T`.
template <typename T>
class OptionalRef {
 public:
  using value_type = T;

  OptionalRef() : OptionalRef(std::nullopt) {}

  OptionalRef(T& value) : value_(&value) {}
  OptionalRef(std::nullopt_t) : value_(nullptr) {}

  OptionalRef(const OptionalRef<T>&) = default;
  OptionalRef(OptionalRef<T>&&) = default;

  OptionalRef<T>& operator=(const OptionalRef<T>&) = default;
  OptionalRef<T>& operator=(OptionalRef<T>&&) = default;

  const T& operator*() const { return *value_; }
  T& operator*() { return *value_; }

  const T* operator->() const { return value_; }
  T* operator->() { return value_; }

  bool operator==(const T& value) const {
    return value_ != nullptr && *value_ == value;
  }

  bool operator==(std::nullopt_t) const { return value_ == nullptr; }

  bool operator==(const OptionalRef<T>& other) const {
    if (value_ == nullptr) return other.value_ == nullptr;
    if (other.value_ == nullptr) return false;
    return *value_ == *other.value_;
  }

  // Returns true if the value contains a reference to an object.
  bool has_value() const { return value_ != nullptr; }

 private:
  T* value_ = nullptr;
};

}  // namespace lucid
