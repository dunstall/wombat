// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/frame.h"

namespace wombat::broker::frame {

class Record : public Frame {
 public:
  Record() = default;

  explicit Record(const std::vector<uint8_t>& data);

  ~Record() override {}

  bool operator==(const Record& record) const;

  std::vector<uint8_t> data() const { return data_; }

  std::vector<uint8_t> Encode() const override;

  static std::optional<Record> Decode(const std::vector<uint8_t>& data);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  std::vector<uint8_t> data_;
};

}  // namespace wombat::broker::frame
