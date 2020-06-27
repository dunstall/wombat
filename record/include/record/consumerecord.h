// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

class ConsumeRecord : public Record {
 public:
  explicit ConsumeRecord(uint32_t offset);

  ~ConsumeRecord() override {}

  bool operator==(const ConsumeRecord& record) const;

  uint32_t offset() const { return offset_; }

  std::vector<uint8_t> Encode() const override;

  static std::optional<ConsumeRecord> Decode(const std::vector<uint8_t>& data);

 private:
  uint32_t offset_;
};

}  // namespace wombat::broker::record
