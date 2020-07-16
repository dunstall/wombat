// Copyright 2020 Andrew Dunstall

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace wombat::broker::log {

constexpr int ID_PADDING = 20;

// Abstract class representing a segment of the log.
class Segment {
 public:
  virtual ~Segment() {}

  Segment(const Segment&) = delete;
  Segment& operator=(const Segment&) = delete;

  Segment(Segment&& segment);
  Segment& operator=(Segment&& segment);

  uint32_t size() const { return size_; }

  bool is_full() const { return size_ >= limit_; }

  void Append(const std::vector<uint8_t>& data);

  std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size);

  uint32_t Send(uint32_t offset, uint32_t size, int fd);

 protected:
  Segment(const std::filesystem::path& path, uint32_t limit);

  uint32_t Size() const;

  std::filesystem::path path_;

  uint32_t size_;

  int fd_;

 private:
  uint32_t limit_;
};

std::string IdToName(uint32_t id);

}  // namespace wombat::broker::log
