// Copyright 2020 Andrew Dunstall

#include "record/message.h"

#include <arpa/inet.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

MessageHeader::MessageHeader(MessageType type,
                             uint32_t partition_id,
                             uint32_t payload_size)
    : type_{type}, partition_id_{partition_id}, payload_size_{payload_size} {
  if (payload_size_ > kLimit || payload_size == 0) {
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
  if (!payload_size || payload_size > kLimit || payload_size == 0) {
    return std::nullopt;
  }

  return MessageHeader{static_cast<MessageType>(*type), *id, *payload_size};
}

Message::Message(MessageType type,
                 uint32_t partition_id,
                 const std::vector<uint8_t>& payload)
    : Message{MessageHeader(type, partition_id, payload.size()), payload} {}

Message::Message(MessageHeader header,
                 const std::vector<uint8_t>& payload)
    : header_{header}, payload_{payload} {}

bool Message::operator==(const Message& message) const {
  return header_ == message.header_
      && payload_ == message.payload_;
}

bool Message::operator!=(const Message& message) const {
  return !(*this == message);
}

std::vector<uint8_t> Message::Encode() const {
  std::vector<uint8_t> enc = header_.Encode();
  enc.insert(enc.end(), payload_.begin(), payload_.end());
  return enc;
}

std::optional<Message> Message::Decode(const std::vector<uint8_t>& enc) {
  std::optional<MessageHeader> header = MessageHeader::Decode(enc);
  if (!header) {
    return std::nullopt;
  }

  if (enc.size() < MessageHeader::kSize + header->payload_size()) {
    return std::nullopt;
  }

  return Message{
    *header,
    std::vector<uint8_t>(
        enc.begin() + MessageHeader::kSize,
        enc.begin() + MessageHeader::kSize + header->payload_size()
    )
  };
}

}  // namespace wombat::broker::record
