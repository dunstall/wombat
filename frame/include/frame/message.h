// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/frame.h"
#include "frame/messageheader.h"

namespace wombat::broker::frame {

// Message represents a frame with a type that can wrap other frames as
// payload.
class Message : public Frame {
 public:
  Message(Type type, uint32_t partition_id,
          const std::vector<uint8_t>& payload);

  Message(MessageHeader header, const std::vector<uint8_t>& payload);

  ~Message() override {}

  bool operator==(const Message& message) const;

  bool operator!=(const Message& message) const;

  Type type() const { return header_.type(); }

  uint32_t partition_id() const { return header_.partition_id(); }

  std::vector<uint8_t> payload() const { return payload_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<Message> Decode(const std::vector<uint8_t>& enc);

 private:
  MessageHeader header_;

  std::vector<uint8_t> payload_;
};

}  // namespace wombat::broker::frame
