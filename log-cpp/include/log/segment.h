#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace wombat::log {

constexpr int ID_PADDING = 20;

// Abstract class representing a segment of the log.
class Segment {
 public:
  virtual ~Segment() {};

  // TODO 
  // Segment(const Segment&) = delete;
  // Segment& operator=(const Segment&) = delete;

  // Segment(Segment&&) = default;
  // Segment& operator=(Segment&&) = default;

  uint64_t size() const { return size_; }

  bool is_full() const { return size_ >= limit_; }

  void Append(const std::vector<uint8_t>& data);

  std::vector<uint8_t> Lookup(uint64_t offset, uint64_t size);

  uint64_t Send(uint64_t offset, uint64_t size, int fd);

  uint64_t Recv(uint64_t size, int fd);

 protected:
  Segment(const std::filesystem::path& path, uint64_t limit);

  uint64_t Size() const;

  std::filesystem::path path_;

  uint64_t size_;

  int fd_;

 private:
  uint64_t limit_;
};

std::string IdToName(uint64_t id);

}  // namespace wombat::log
