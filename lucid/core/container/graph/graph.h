#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <vector>

namespace lucid {

// A range of vertices of type `V`.
template <typename R, typename V>
concept VertexRange = std::ranges::input_range<R> and
                      std::same_as<std::ranges::range_value_t<R>, V>;

// A single-source, single-sink graph.
template <typename G>
concept Graph = requires(G g, G::vertex_type v) {
  { VertexCount(g) } -> std::same_as<std::size_t>;
  { Vertices(g) } -> std::same_as<std::vector<typename G::vertex_type>>;
  { SourceVertex(g) } -> std::same_as<typename G::vertex_type>;
  { SinkVertex(g) } -> std::same_as<typename G::vertex_type>;
  { NextVertices(g, v) } -> VertexRange<typename G::vertex_type>;
  { PrevVertices(g, v) } -> VertexRange<typename G::vertex_type>;

  { VertexId(g, v) } -> std::same_as<std::uint32_t>;
};

}  // namespace lucid
