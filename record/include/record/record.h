#pragma once

#include <cstdint>
#include <vector>

namespace wombat::broker {

class Record {
 public:
  virtual ~Record() {}

  virtual std::vector<uint8_t> Encode() const = 0;
};

}  // namespace wombat::broker
