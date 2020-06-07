#pragma once

#include <string>
#include <vector>

namespace wombat {
namespace log {

template<class S>
class Log {
 public:
  Log(size_t segment_limit, std::string path);

  void Append(const std::vector<uint8_t>& data) {
  }

  std::vector<uint8_t> lookup(uint64_t offset, uint64_t size) const {
    return std::vector<uint8_t>{};
  }
};

}  // namespace log
}  // namespace wombat
