// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

class StatResponse {
 public:
  explicit StatResponse(uint32_t size);

  ~StatResponse() {}

  bool operator==(const StatResponse& record) const;

  uint32_t size() const { return size_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<StatResponse> Decode(const std::vector<uint8_t>& data);

 private:
  uint32_t size_;
};

}  // namespace wombat::broker::record
