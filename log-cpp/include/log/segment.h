#pragma once

#include <string>
#include <vector>

namespace wombat::log {

constexpr int ID_PADDING = 20;

class Segment {
 public:
  Segment(uint64_t limit);

  virtual ~Segment() {};

  uint64_t size() const { return size_; }

  bool is_full() const { return size_ >= limit_; }

  virtual void Append(const std::vector<uint8_t>& data) = 0;

  virtual std::vector<uint8_t> Lookup(uint64_t offset, uint64_t size) = 0;

 protected:
  uint64_t size_;

 private:
  uint64_t limit_;
};

std::string IdToName(uint64_t id);

}  // namespace wombat::log
