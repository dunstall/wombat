// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wombat::broker::connection {

class Socket {
 public:
  Socket() = default;

  virtual ~Socket() {}

  Socket(const Socket& conn) = delete;
  Socket& operator=(const Socket& conn) = delete;

  Socket(Socket&& conn) = default;
  Socket& operator=(Socket&& conn) = default;

  virtual void Connect(const std::string& ip, uint16_t port) = 0;

  virtual size_t Read(const std::vector<uint8_t>* buf, size_t from) = 0;

  virtual size_t Write(const std::vector<uint8_t>& buf, size_t from) = 0;
};

}  // namespace wombat::broker::connection
