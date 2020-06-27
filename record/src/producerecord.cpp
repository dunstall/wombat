// Copyright 2020 Andrew Dunstall

#include "record/producerecord.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "record/record.h"

namespace wombat::broker {

constexpr uint32_t ProduceRecord::kLimit;

ProduceRecord::ProduceRecord(const std::vector<uint8_t>& data)
    : data_{data} {
  if (data_.size() > kLimit) {
    throw std::invalid_argument{"data size exceeds limit"};
  }
}

bool ProduceRecord::operator==(const ProduceRecord& record) const {
  return data_ == record.data_;
}

std::vector<uint8_t> ProduceRecord::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(data_.size());
  enc.insert(enc.end(), data_.begin(), data_.end());
  return enc;
}

std::optional<ProduceRecord> ProduceRecord::Decode(
    const std::vector<uint8_t>& enc) {
  std::optional<uint32_t> size = DecodeU32(enc);
  if (!size || *size > kLimit) {
    return std::nullopt;
  }
  if (*size > enc.size() - sizeof(uint32_t)) {
    return std::nullopt;
  }

  std::vector<uint8_t> data(
      enc.begin() + sizeof(uint32_t),
      enc.begin() + sizeof(uint32_t) + *size
  );
  return std::optional<ProduceRecord>{data};
}

}  // namespace wombat::broker
