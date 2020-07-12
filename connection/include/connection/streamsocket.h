// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "connection/socket.h"

namespace wombat::broker::connection {

class StreamSocket : public Socket {
 public:
  ~StreamSocket() override;

  StreamSocket(const StreamSocket& sock) = delete;
  StreamSocket& operator=(const StreamSocket& sock) = delete;

  StreamSocket(StreamSocket&& sock);
  StreamSocket& operator=(StreamSocket&& sock);

  void Connect(const std::string& ip, uint16_t port) override;

  size_t Read(const std::vector<uint8_t>* buf, size_t from) override;

  size_t Write(const std::vector<uint8_t>& buf, size_t from) override;
};

}  // namespace wombat::broker::connection
