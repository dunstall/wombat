// Copyright 2020 Andrew Dunstall

#include "record/recordrequest.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

RecordRequest::RecordRequest(uint32_t offset) : offset_{offset} {}

bool RecordRequest::operator==(const RecordRequest& record) const {
  return offset_ == record.offset_;
}

std::vector<uint8_t> RecordRequest::Encode() const {
  return EncodeU32(offset_);
}

std::optional<RecordRequest> RecordRequest::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> offset = DecodeU32(enc);
  if (!offset) {
    return std::nullopt;
  }
  return std::optional<RecordRequest>{*offset};
}

}  // namespace wombat::broker::record
