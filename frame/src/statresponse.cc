// Copyright 2020 Andrew Dunstall

#include "frame/statresponse.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/record.h"

namespace wombat::broker::record {

StatResponse::StatResponse(uint32_t size) : size_{size} {}

bool StatResponse::operator==(const StatResponse& record) const {
  return size_ == record.size_;
}

std::vector<uint8_t> StatResponse::Encode() const {
  return EncodeU32(size_);
}

std::optional<StatResponse> StatResponse::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> size = DecodeU32(enc);
  if (!size) {
    return std::nullopt;
  }
  return std::optional<StatResponse>{*size};
}

}  // namespace wombat::broker::record
