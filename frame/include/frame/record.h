// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace wombat::broker::record {

class Record {
 public:
  Record() = default;

  explicit Record(const std::vector<uint8_t>& data);

  ~Record() {}

  bool operator==(const Record& record) const;

  std::vector<uint8_t> data() const { return data_; }

  std::vector<uint8_t> Encode() const;

  static std::optional<Record> Decode(const std::vector<uint8_t>& data);

 private:
  // Maximum record data size.
  static constexpr uint32_t kLimit = 512;

  std::vector<uint8_t> data_;
};

std::vector<uint8_t> EncodeU32(uint32_t n);

std::optional<uint32_t> DecodeU32(const std::vector<uint8_t>& enc);

}  // namespace wombat::broker::record
