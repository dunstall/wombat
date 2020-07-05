// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>
#include <filesystem>

#include "log/segmentfactory.h"

namespace wombat::broker::log {

class SystemSegmentFactory : public SegmentFactory {
 public:
  SystemSegmentFactory(const std::filesystem::path& dir, uint32_t limit);

  ~SystemSegmentFactory() override {}

  std::shared_ptr<Segment> Open(uint32_t id) const override;

 private:
  std::filesystem::path dir_;

  uint32_t limit_;
};

}  // namespace wombat::broker::log
