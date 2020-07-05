// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "log/segment.h"

namespace wombat::broker::log {

class Offsets {
 public:
  explicit Offsets(std::shared_ptr<Segment> segment);

  bool Lookup(uint32_t offset, uint32_t* id, uint32_t* start);

  uint32_t MaxOffset();

  void Insert(uint32_t offset, uint32_t id);

 private:
  bool LoadOffset(uint32_t offset);

  void WriteU32(uint32_t n);

  bool ReadU32(uint32_t offset, uint32_t* n);

  std::map<uint32_t, uint32_t> offsets_;

  std::shared_ptr<Segment> segment_;
};

}  // namespace wombat::broker::log
