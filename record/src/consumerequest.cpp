// Copyright 2020 Andrew Dunstall

#include "record/consumerequest.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

ConsumeRequest::ConsumeRequest(uint32_t offset) : offset_{offset} {}

bool ConsumeRequest::operator==(const ConsumeRequest& record) const {
  return offset_ == record.offset_;
}

std::vector<uint8_t> ConsumeRequest::Encode() const {
  return EncodeU32(offset_);
}

std::optional<ConsumeRequest> ConsumeRequest::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> offset = DecodeU32(enc);
  if (!offset) {
    return std::nullopt;
  }
  return std::optional<ConsumeRequest>{*offset};
}

}  // namespace wombat::broker::record
