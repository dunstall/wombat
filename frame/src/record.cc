// Copyright 2020 Andrew Dunstall

#include "frame/record.h"

#include <arpa/inet.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

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

std::vector<uint8_t> EncodeU32(uint32_t n) {
  std::vector<uint8_t> enc(sizeof(uint32_t));
  n = htonl(n);
  std::memcpy(enc.data(), &n, sizeof(uint32_t));
  return enc;
}

std::optional<uint32_t> DecodeU32(const std::vector<uint8_t>& enc) {
  if (enc.size() < sizeof(uint32_t)) {
    return std::nullopt;
  }

  uint32_t n;
  std::memcpy(&n, enc.data(), sizeof(uint32_t));
  return ntohl(n);
}

}  // namespace wombat::broker
