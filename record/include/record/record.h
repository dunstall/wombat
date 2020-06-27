// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker {

class Record {
 public:
  virtual ~Record() {}

  virtual std::vector<uint8_t> Encode() const = 0;
};

std::vector<uint8_t> EncodeU32(uint32_t n);

std::optional<uint32_t> DecodeU32(const std::vector<uint8_t>& enc);

}  // namespace wombat::broker
