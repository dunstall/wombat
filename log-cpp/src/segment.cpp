#include "log/segment.h"

namespace wombat::log {

Segment::Segment(uint64_t limit) : size_{0}, limit_{limit} {}

std::string IdToName(uint64_t id) {
  std::string id_str = std::to_string(id);
  return "segment-" + std::string(ID_PADDING - id_str.length(), '0') + id_str;
}

}  // namespace wombat::log
