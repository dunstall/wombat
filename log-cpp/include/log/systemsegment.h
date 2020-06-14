#pragma once

#include <cstdint>
#include <filesystem>

#include "log/segment.h"

namespace wombat::log {

class SystemSegment : public Segment {
 public:
  SystemSegment(uint64_t id, const std::filesystem::path& dir, size_t limit);

  ~SystemSegment() override;

  // TODO 
  // Segment(const Segment&) = delete;
  // Segment& operator=(const Segment&) = delete;

  // Segment(Segment&&) = default;
  // Segment& operator=(Segment&&) = default;
};

}  // namespace wombat::log
