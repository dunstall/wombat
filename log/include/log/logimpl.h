// Copyright 2020 Andrew Dunstall

#pragma once

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "log/offsets.h"
#include "log/log.h"
#include "log/segmentfactory.h"

namespace wombat::broker::log {

constexpr uint32_t OFFSET_SEGMENT_ID = 0;

// TODO(AD) Mock segments to unit test
class LogImpl : public Log {
 public:
  LogImpl(std::unique_ptr<SegmentFactory> segment_factory);

  virtual ~LogImpl() {}

  LogImpl(const LogImpl&) = delete;
  LogImpl& operator=(const LogImpl&) = delete;

  LogImpl(LogImpl&&) = default;
  LogImpl& operator=(LogImpl&&) = default;

  uint32_t size() const { return size_; }

  void Append(const std::vector<uint8_t>& data) override;

  std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) override;

  uint32_t Send(uint32_t offset, uint32_t size, int fd) override;

 private:
  std::shared_ptr<Segment> LookupSegment(uint32_t id);

  Offsets offsets_;

  std::filesystem::path path_;

  std::unordered_map<uint32_t, std::shared_ptr<Segment>> segments_;

  uint32_t active_;

  uint32_t segment_limit_;

  uint32_t size_;

  std::unique_ptr<SegmentFactory> segment_factory_;
};

}  // namespace wombat::broker::log
