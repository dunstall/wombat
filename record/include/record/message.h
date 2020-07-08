// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

enum class MessageType : uint32_t {
  kProduceRequest,
  kConsumeRequest,
  kReplicaRequest,
  kConsumeResponse,
  kReplicaResponse
};

class MessageHeader {
 public:
  static constexpr int kSize = 8;

  MessageHeader(MessageType type,
                uint32_t partition_id,
                uint32_t payload_size);

  bool operator==(const MessageHeader& header) const;

  bool operator!=(const MessageHeader& header) const;

  MessageType type() const { return type_; }

  uint32_t partition_id() const { return partition_id_; }

  uint32_t payload_size() const { return payload_size_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<MessageHeader> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  MessageType type_;

  uint32_t partition_id_;

  uint32_t payload_size_;
};

class Message {
 public:
  Message(MessageType type,
          uint32_t partition_id,
          const std::vector<uint8_t>& payload);

  bool operator==(const Message& request) const;

  bool operator!=(const Message& request) const;

  MessageType type() const { return type_; }

  uint32_t partition_id() const { return partition_id_; }

  std::vector<uint8_t> payload() const { return payload_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<Message> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  MessageType type_;

  uint32_t partition_id_;

  std::vector<uint8_t> payload_;
};

}  // namespace wombat::broker::record
