#include "lucid/writer.h"

#include <cstdio>
#include <string>
#include <string_view>

namespace lucid {

Writer StringWriter(std::string& out) {
  return [&out](std::string_view s) { out.append(s); };
}

Writer FileWriter(std::FILE* out) {
  return [out](std::string_view s) { std::fwrite(s.data(), s.size(), 1, out); };
}

}  // namespace lucid
