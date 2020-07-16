// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/frame.h"

namespace wombat::broker::frame {

// Type indicates type of frame in the message payload.
enum class Type : uint32_t {
  kProduceRequest,
  kConsumeRequest,
  kReplicaRequest,
  kConsumeResponse,
  kReplicaResponse,
  kStatRequest,
  kStatResponse,
  kDummy
};

class MessageHeader : public Frame {
 public:
  static constexpr int kSize = 12;

  MessageHeader(Type type, uint32_t partition_id, uint32_t payload_size);

  ~MessageHeader() override {}

  bool operator==(const MessageHeader& header) const;

  bool operator!=(const MessageHeader& header) const;

  Type type() const { return type_; }

  uint32_t partition_id() const { return partition_id_; }

  uint32_t payload_size() const { return payload_size_; }

  std::vector<uint8_t> Encode() const override;

  static std::optional<MessageHeader> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  Type type_;

  uint32_t partition_id_;

  uint32_t payload_size_;
};

}  // namespace wombat::broker::frame
