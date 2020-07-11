// Copyright 2020 Andrew Dunstall

#include "frame/offset.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/utils.h"

namespace wombat::broker {

Offset::Offset(uint32_t offset) : offset_{offset} {}

bool Offset::operator==(const Offset& record) const {
  return offset_ == record.offset_;
}

bool Offset::operator!=(const Offset& record) const {
  return !(*this == record);
}

std::vector<uint8_t> Offset::Encode() const {
  return EncodeU32(offset_);
}

std::optional<Offset> Offset::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> offset = DecodeU32(enc);
  if (!offset) {
    return std::nullopt;
  }
  return std::optional<Offset>{*offset};
}

}  // namespace wombat::broker
