// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker::record {

class ProduceRecord : public Record {
 public:
  explicit ProduceRecord(const std::vector<uint8_t>& data);

  ~ProduceRecord() override {}

  bool operator==(const ProduceRecord& record) const;

  std::vector<uint8_t> data() const { return data_; }

  std::vector<uint8_t> Encode() const override;

  static std::optional<ProduceRecord> Decode(const std::vector<uint8_t>& data);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  std::vector<uint8_t> data_;
};

}  // namespace wombat::broker::record
