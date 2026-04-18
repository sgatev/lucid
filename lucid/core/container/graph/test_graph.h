#pragma once

#include <cstddef>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

class TestGraph {
 public:
  using vertex_type = char;

  void SetSource(char v) {
    AddVertex(v);

    source_ = v;
  }

  void SetSink(char v) {
    AddVertex(v);

    sink_ = v;
  }

  void AddEdge(char from, char to) {
    AddVertex(from);
    AddVertex(to);

    nexts_.Get(from)->Insert(to);
    prevs_.Get(to)->Insert(from);
  }

  const std::vector<char>& Vertices() const { return vertices_; }

 private:
  friend std::size_t VertexCount(const TestGraph&);
  friend std::vector<char> Vertices(const TestGraph&);
  friend char SourceVertex(const TestGraph&);
  friend char SinkVertex(const TestGraph&);
  friend std::vector<char> NextVertices(const TestGraph&, char);
  friend std::vector<char> PrevVertices(const TestGraph&, char);
  friend std::uint32_t VertexId(const TestGraph&, TestGraph::vertex_type);

  void AddVertex(char v) {
    if (order_.Get(v).has_value()) return;

    vertices_.push_back(v);
    order_.Insert(v, order_.size());
    nexts_.Insert(v, HashSet<char>());
    prevs_.Insert(v, HashSet<char>());
  }

  char source_;
  char sink_;
  std::vector<char> vertices_;
  HashMap<char, HashSet<char>> nexts_;
  HashMap<char, HashSet<char>> prevs_;
  HashMap<char, std::uint32_t> order_;
};

inline std::size_t VertexCount(const TestGraph& g) {
  return g.vertices_.size();
}

inline std::vector<char> Vertices(const TestGraph& g) { return g.vertices_; }

inline char SourceVertex(const TestGraph& g) { return g.source_; }

inline char SinkVertex(const TestGraph& g) { return g.sink_; }

inline std::vector<char> NextVertices(const TestGraph& g, char v) {
  std::vector<char> next_vertices;
  if (auto it = g.nexts_.Get(v); it.has_value()) {
    for (char n : *it) next_vertices.push_back(n);
  }
  return next_vertices;
}

inline std::vector<char> PrevVertices(const TestGraph& g, char v) {
  std::vector<char> prev_vertices;
  if (auto it = g.prevs_.Get(v); it.has_value()) {
    for (char n : *it) prev_vertices.push_back(n);
  }
  return prev_vertices;
}

inline std::uint32_t VertexId(const TestGraph& g, char v) {
  return *g.order_.Get(v);
}

}  // namespace lucid
