#pragma once

#include <poll.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "server/connection.h"

namespace wombat::log {

// TODO(AD) Only handling produce for now
struct Request {
  std::vector<uint8_t> data;
};

class Server {
 public:
  Server(uint16_t port, int max_clients = 1024);

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = default;
  Server& operator=(Server&& conn) = default;

  std::vector<Request> Poll();

 private:
  static const int kListenBacklog = 10;

  void Listen();

  void Accept();

  void Read(int i);

  bool PendingConnection() const;

  bool PendingRead(int i) const;

  bool PendingWrite(int i) const;

  uint16_t port_;

  int listenfd_;

  std::vector<struct pollfd> fds_;

  int max_fd_index_ = 0;

  std::unordered_map<int, Connection> connections_;

  int max_clients_;
};

}  // namespace wombat::log