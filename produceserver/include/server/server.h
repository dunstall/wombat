// Copyright 2020 Andrew Dunstall

#pragma once

#include <poll.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "record/producerecord.h"
#include "server/connection.h"

namespace wombat::broker::produceserver {

class Server {
 public:
  explicit Server(uint16_t port, int max_clients = 1024);

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = default;
  Server& operator=(Server&& conn) = default;

  std::vector<record::ProduceRecord> Poll();

 private:
  static const int kListenBacklog = 10;

  void Listen();

  void Accept();

  std::vector<record::ProduceRecord> Read(int i);

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

}  // namespace wombat::broker::produceserver
