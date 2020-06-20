#pragma once

#include <cstdint>
#include <filesystem>

#include "log/segment.h"

namespace wombat::log {

class SystemSegment : public Segment {
 public:
  SystemSegment(uint64_t id, const std::filesystem::path& dir, size_t limit);

  ~SystemSegment() override;
};

}  // namespace wombat::log
