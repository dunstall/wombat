// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <vector>

namespace wombat::broker {

// Represents arbitrary data that can be encoded and decoding by prefixing the
// size.
class Frame {
 public:
  virtual ~Frame() {}

  virtual std::vector<uint8_t> Encode() const = 0;
};

}  // namespace wombat::broker
