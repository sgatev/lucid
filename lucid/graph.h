#include <concepts>
#include <cstddef>
#include <vector>

namespace lucid {

// A single-source, single-sink graph.
template <typename G>
concept Graph = requires(G g, G::vertex_type v) {
  { VertexCount(g) } -> std::same_as<std::size_t>;
  { SourceVertex(g) } -> std::same_as<typename G::vertex_type>;
  { SinkVertex(g) } -> std::same_as<typename G::vertex_type>;
  { NextVertices(g, v) } -> std::same_as<std::vector<typename G::vertex_type>>;
  { PrevVertices(g, v) } -> std::same_as<std::vector<typename G::vertex_type>>;

  { v.id() } -> std::same_as<std::uint32_t>;
};

}  // namespace lucid
