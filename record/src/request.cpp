// Copyright 2020 Andrew Dunstall

#include "record/request.h"

#include <arpa/inet.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

RequestHeader::RequestHeader(RequestType type, uint32_t payload_size)
    : type_{type}, payload_size_{payload_size} {
  if (payload_size_ > kLimit) {
    throw std::invalid_argument{"payload size exceeds limit"};
  }
}

bool RequestHeader::operator==(const RequestHeader& header) const {
  return type_ == header.type_ && payload_size_ == header.payload_size_;
}

bool RequestHeader::operator!=(const RequestHeader& header) const {
  return !(*this == header);
}

std::vector<uint8_t> RequestHeader::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(static_cast<uint32_t>(type_));
  std::vector<uint8_t> size = EncodeU32(payload_size_);
  enc.insert(enc.end(), size.begin(), size.end());
  return enc;
}

std::optional<RequestHeader> RequestHeader::Decode(const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> type = DecodeU32(enc);
  if (!type) {
    return std::nullopt;
  }

  std::optional<uint32_t> payload_size = DecodeU32(
      std::vector<uint8_t>(enc.begin() + sizeof(uint32_t), enc.end())
  );
  if (!payload_size) {
    return std::nullopt;
  }

  return RequestHeader{static_cast<RequestType>(*type), *payload_size};
}

Request::Request(RequestType type, const std::vector<uint8_t>& payload)
    : type_{type}, payload_{payload} {
  if (payload_.size() > kLimit) {
    throw std::invalid_argument{"payload size exceeds limit"};
  }
}

bool Request::operator==(const Request& request) const {
  return type_ == request.type_ && payload_ == request.payload_;
}

bool Request::operator!=(const Request& request) const {
  return !(*this == request);
}

std::vector<uint8_t> Request::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(static_cast<uint32_t>(type_));
  std::vector<uint8_t> size = EncodeU32(payload_.size());
  enc.insert(enc.end(), size.begin(), size.end());
  enc.insert(enc.end(), payload_.begin(), payload_.end());
  return enc;
}

std::optional<Request> Request::Decode(const std::vector<uint8_t>& enc) {
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

  return Request{
    static_cast<RequestType>(*type),
    std::vector<uint8_t>(
        enc.begin() + (sizeof(uint32_t) * 2),
        enc.begin() + (sizeof(uint32_t) * 2) + *size
    )
  };
}

}  // namespace wombat::broker::record
