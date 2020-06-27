#include "record/record.h"

#include <arpa/inet.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

namespace wombat::broker {

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
