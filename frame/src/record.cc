// Copyright 2020 Andrew Dunstall

#include "frame/record.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

#include "frame/utils.h"

namespace wombat::broker {

constexpr uint32_t Record::kLimit;

Record::Record(const std::vector<uint8_t>& data)
    : data_{data} {
  if (data_.size() > kLimit) {
    throw std::invalid_argument{"data size exceeds limit"};
  }
}

bool Record::operator==(const Record& record) const {
  return data_ == record.data_;
}

std::vector<uint8_t> Record::Encode() const {
  std::vector<uint8_t> enc = EncodeU32(data_.size());
  enc.insert(enc.end(), data_.begin(), data_.end());
  return enc;
}

std::optional<Record> Record::Decode(
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
  return std::optional<Record>{data};
}

}  // namespace wombat::broker
