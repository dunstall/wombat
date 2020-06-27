#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker {

class ProduceRecord : public Record {
 public:
  ~ProduceRecord() override {}

  std::vector<uint8_t> Encode() const override;

  static std::optional<ProduceRecord> Decode(const std::vector<uint8_t>& data);
};

}  // namespace wombat::broker
