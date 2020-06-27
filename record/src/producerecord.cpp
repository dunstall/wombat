#include "record/producerecord.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker {

std::vector<uint8_t> ProduceRecord::Encode() const {
  return {};
}

std::optional<ProduceRecord> ProduceRecord::Decode(
    const std::vector<uint8_t>& data)
{
  return std::optional<ProduceRecord>{};
}

}  // namespace wombat::broker
