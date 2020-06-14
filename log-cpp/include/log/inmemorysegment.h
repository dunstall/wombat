#pragma once

#include <filesystem>
#include <vector>

#include "log/segment.h"

namespace wombat::log {

// Implements Segment using an in-memory buffer. This is for testing only as
// uses globals (singletons) to manage state.
class InMemorySegment : public Segment {
 public:
  InMemorySegment(uint64_t id, const std::filesystem::path& path, size_t limit);

  ~InMemorySegment() override {}

  void Append(const std::vector<uint8_t>& data) override;

  std::vector<uint8_t> Lookup(uint64_t offset, uint64_t size) override;

  uint64_t Send(uint64_t offset, uint64_t size, int fd) override {
    // TODO(AD)
    return 0;
  }

  uint64_t Recv(uint64_t size, int fd) override {
    // TODO(AD)
    return 0;
  }

 private:
  std::filesystem::path path_;
};

}  // namespace wombat::log
