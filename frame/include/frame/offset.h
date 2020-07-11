// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/frame.h"

namespace wombat::broker {

class Offset : public Frame {
 public:
  explicit Offset(uint32_t offset);

  ~Offset() override {}

  bool operator==(const Offset& record) const;

  bool operator!=(const Offset& record) const;

  uint32_t offset() const { return offset_; }

  std::vector<uint8_t> Encode() const override;

  static std::optional<Offset> Decode(const std::vector<uint8_t>& data);

 private:
  uint32_t offset_;
};

}  // namespace wombat::broker
