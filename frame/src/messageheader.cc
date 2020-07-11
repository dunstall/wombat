// Copyright 2020 Andrew Dunstall

#include "frame/messageheader.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/utils.h"

namespace wombat::broker {

MessageHeader::MessageHeader(Type type,
                             uint32_t partition_id,
                             uint32_t payload_size)
    : type_{type}, partition_id_{partition_id}, payload_size_{payload_size} {
  if (payload_size_ > kLimit) {
    throw std::invalid_argument{"invalid payload size"};
  }
}

bool MessageHeader::operator==(const MessageHeader& header) const {
  return type_ == header.type_
      && partition_id_ == header.partition_id_
      && payload_size_ == header.payload_size_;
}

bool MessageHeader::operator!=(const MessageHeader& header) const {
  return !(*this == header);
}

std::vector<uint8_t> MessageHeader::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(static_cast<uint32_t>(type_));
  std::vector<uint8_t> id = EncodeU32(partition_id_);
  enc.insert(enc.end(), id.begin(), id.end());
  std::vector<uint8_t> size = EncodeU32(payload_size_);
  enc.insert(enc.end(), size.begin(), size.end());
  return enc;
}

std::optional<MessageHeader> MessageHeader::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> type = DecodeU32(enc);
  if (!type) {
    return std::nullopt;
  }

  std::optional<uint32_t> id = DecodeU32(
      std::vector<uint8_t>(enc.begin() + sizeof(uint32_t), enc.end())
  );
  if (!id) {
    return std::nullopt;
  }

  std::optional<uint32_t> payload_size = DecodeU32(
      std::vector<uint8_t>(
          enc.begin() + sizeof(uint32_t) + sizeof(uint32_t), enc.end()
      )
  );
  if (!payload_size || payload_size > kLimit) {
    return std::nullopt;
  }

  return MessageHeader{static_cast<Type>(*type), *id, *payload_size};
}

}  // namespace wombat::broker
