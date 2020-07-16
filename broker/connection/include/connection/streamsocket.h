// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "connection/socket.h"

namespace wombat::broker::connection {

class StreamSocket : public Socket {
 public:
  explicit StreamSocket(int sockfd);

  ~StreamSocket() override;

  StreamSocket(const StreamSocket& sock) = delete;
  StreamSocket& operator=(const StreamSocket& sock) = delete;

  StreamSocket(StreamSocket&& sock);
  StreamSocket& operator=(StreamSocket&& sock);

  void Connect(const std::string& ip, uint16_t port) override;

  size_t Read(std::vector<uint8_t>* buf, size_t from, size_t n) override;

  size_t Write(const std::vector<uint8_t>& buf, size_t from, size_t n) override;

  std::optional<int> sockfd_ = std::nullopt;
};

}  // namespace wombat::broker::connection
