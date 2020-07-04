// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

enum class ResponseType : uint32_t {
  kConsumeResponse,
};

class Response {
 public:
  Response(ResponseType type, const std::vector<uint8_t>& payload) {}
};

}  // namespace wombat::broker::record
