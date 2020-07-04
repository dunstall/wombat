// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

class ConsumeRequest {
 public:
  explicit ConsumeRequest(uint32_t offset);

  ~ConsumeRequest() {}

  bool operator==(const ConsumeRequest& record) const;

  uint32_t offset() const { return offset_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<ConsumeRequest> Decode(const std::vector<uint8_t>& data);

 private:
  uint32_t offset_;
};

}  // namespace wombat::broker::record
