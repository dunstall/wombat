// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

enum class RequestType : uint32_t {
  kProduceRecord,
  kConsumeRecord,
  // TODO(AD) Replica/leader
};

class RequestHeader {
 public:
  static constexpr int kSize = 8;

  RequestHeader(RequestType type, uint32_t payload_size);

  bool operator==(const RequestHeader& header) const;

  bool operator!=(const RequestHeader& header) const;

  RequestType type() const { return type_; }

  uint32_t payload_size() const { return payload_size_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<RequestHeader> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  RequestType type_;

  uint32_t payload_size_;
};

// TODO(AD) Remove?
class Request {
 public:
  Request(RequestType type, const std::vector<uint8_t>& payload);

  bool operator==(const Request& request) const;

  bool operator!=(const Request& request) const;

  RequestType type() const { return type_; }

  std::vector<uint8_t> payload() const { return payload_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<Request> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  RequestType type_;

  std::vector<uint8_t> payload_;
};

}  // namespace wombat::broker::record
