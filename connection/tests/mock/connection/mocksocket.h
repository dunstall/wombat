// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "connection/socket.h"
#include "gmock/gmock.h"

namespace wombat::broker::connection {

class MockSocket : public Socket {
 public:
  MOCK_METHOD(void, Connect, (const std::string& ip, uint16_t port),
              (override));
  MOCK_METHOD(size_t, Read, (std::vector<uint8_t> * buf, size_t from, size_t n),
              (override));
  MOCK_METHOD(size_t, Write, (const std::vector<uint8_t>& buf, size_t from),
              (override));
};

}  // namespace wombat::broker::connection
