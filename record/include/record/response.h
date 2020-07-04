// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

enum class ResponseType : uint32_t {
  kProduce,
  kConsume
};

class Response {
 public:
  Response(ResponseType type, const std::vector<uint8_t>& payload);

  bool operator==(const Response& response) const;

  bool operator!=(const Response& response) const;

  ResponseType type() const { return type_; }

  std::vector<uint8_t> payload() const { return payload_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<Response> Decode(const std::vector<uint8_t>& enc);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  ResponseType type_;

  std::vector<uint8_t> payload_;
};

}  // namespace wombat::broker::record
