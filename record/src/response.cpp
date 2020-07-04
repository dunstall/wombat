// Copyright 2020 Andrew Dunstall

#include "record/response.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

Response::Response(ResponseType type, const std::vector<uint8_t>& payload)
    : type_{type}, payload_{payload} {
  if (payload_.size() > kLimit) {
    throw std::invalid_argument{"payload size exceeds limit"};
  }
}

bool Response::operator==(const Response& response) const {
  return type_ == response.type_ && payload_ == response.payload_;
}

bool Response::operator!=(const Response& response) const {
  return !(*this == response);
}

std::vector<uint8_t> Response::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(static_cast<uint32_t>(type_));
  std::vector<uint8_t> size = EncodeU32(payload_.size());
  enc.insert(enc.end(), size.begin(), size.end());
  enc.insert(enc.end(), payload_.begin(), payload_.end());
  return enc;
}

std::optional<Response> Response::Decode(const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> type = DecodeU32(enc);
  if (!type) {
    return std::nullopt;
  }

  std::optional<uint32_t> size = DecodeU32(
      std::vector<uint8_t>(enc.begin() + sizeof(uint32_t), enc.end())
  );
  if (!size) {
    return std::nullopt;
  }

  if (enc.size() < (sizeof(uint32_t) * 2) + *size) {
    return std::nullopt;
  }

  return Response{
    static_cast<ResponseType>(*type),
    std::vector<uint8_t>(
        enc.begin() + (sizeof(uint32_t) * 2),
        enc.begin() + (sizeof(uint32_t) * 2) + *size
    )
  };
}

}  // namespace wombat::broker::record
