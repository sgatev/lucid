#pragma once

namespace lucid {

// Executes `f.<I>()` for all `I` up to `N`.
// See https://indii.org/blog/cplusplus-static-for-loop.
template <int I, int N, typename F>
constexpr void StaticFor(F f) {
  if constexpr (I < N) {
    f.template operator()<I>();
    StaticFor<I + 1, N>(f);
  }
}

}  // namespace lucid
