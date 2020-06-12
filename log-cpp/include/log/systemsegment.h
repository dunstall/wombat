#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "log/segment.h"

namespace wombat::log {

class SystemSegment : public Segment {
 public:
  SystemSegment(uint64_t id, const std::filesystem::path& path, size_t limit);

  ~SystemSegment() override {}

  void Append(const std::vector<uint8_t>& data) override;

  std::vector<uint8_t> Lookup(uint64_t offset, uint64_t size) override;

 private:
  std::fstream fs_;
};

}  // namespace wombat::log
