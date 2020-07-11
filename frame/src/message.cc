// Copyright 2020 Andrew Dunstall

#include "frame/message.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/utils.h"

namespace wombat::broker {

Message::Message(Type type,
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

}  // namespace wombat::broker
