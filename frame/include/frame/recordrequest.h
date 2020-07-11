// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker {

class RecordRequest {
 public:
  explicit RecordRequest(uint32_t offset);

  ~RecordRequest() {}

  bool operator==(const RecordRequest& record) const;

  uint32_t offset() const { return offset_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<RecordRequest> Decode(const std::vector<uint8_t>& data);

 private:
  uint32_t offset_;
};

}  // namespace wombat::broker
