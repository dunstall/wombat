// Copyright 2020 Andrew Dunstall

#include "record/consumerecord.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

ConsumeRecord::ConsumeRecord(uint32_t offset) : offset_{offset} {}

bool ConsumeRecord::operator==(const ConsumeRecord& record) const {
  return offset_ == record.offset_;
}

std::vector<uint8_t> ConsumeRecord::Encode() const {
  return EncodeU32(offset_);
}

std::optional<ConsumeRecord> ConsumeRecord::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> offset = DecodeU32(enc);
  if (!offset) {
    return std::nullopt;
  }
  return std::optional<ConsumeRecord>{*offset};
}

}  // namespace wombat::broker::record
