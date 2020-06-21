#pragma once

#include <cstdint>
#include <filesystem>

#include "log/segment.h"

namespace wombat::log {

// Implements Segment using an in-memory buffer. This is for testing only as
// uses globals (singletons) to manage state.
class InMemorySegment : public Segment {
 public:
  InMemorySegment(uint32_t id, const std::filesystem::path& dir, uint32_t limit);

  ~InMemorySegment() override;
};

}  // namespace wombat::log
