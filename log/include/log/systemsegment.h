// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <filesystem>

#include "log/segment.h"

namespace wombat::broker {

class SystemSegment : public Segment {
 public:
  SystemSegment(uint32_t id, const std::filesystem::path& dir, uint32_t limit);

  ~SystemSegment() override;

  SystemSegment(const SystemSegment&) = delete;
  SystemSegment& operator=(const SystemSegment&) = delete;

  SystemSegment(SystemSegment&& segment) = default;
  SystemSegment& operator=(SystemSegment&& segment) = default;
};

}  // namespace wombat::broker
