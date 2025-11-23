#include "lucid/reg.h"

#include <unordered_map>
#include <unordered_set>

#include "lucid/string_index.h"

namespace lucid {

std::unordered_map<StringIndex::Ref, std::unordered_set<StringIndex::Ref>>
BuildInterferenceGraph() {
  std::unordered_map<StringIndex::Ref, std::unordered_set<StringIndex::Ref>>
      graph;
  // TODO: implement based on liveness sets
  return graph;
}

}  // namespace lucid
