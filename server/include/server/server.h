// Copyright 2020 Andrew Dunstall

#pragma once

#include <poll.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "event/connection.h"
#include "event/event.h"
#include "util/pollable.h"

namespace wombat::broker::server {

// Server handles reading requests from connections to clients. This does
// not write to the clients (thats left to responder).
class Server : public util::Pollable {
 public:
  explicit Server(uint16_t port, int max_clients = 1024);

  ~Server() override {}

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = delete;
  Server& operator=(Server&& conn) = delete;

  std::shared_ptr<EventQueue> events() const { return event_queue_; }

  void Poll() override;

 private:
  static constexpr int kListenBacklog = 10;
  static constexpr int kPollTimeoutMS = 1000;

  void Listen();

  void Accept();

  void Read(int i);

  bool PendingConnection() const;

  bool PendingRead(int i) const;

  bool PendingWrite(int i) const;  // TODO(AD) Redundent

  int listenfd_;

  std::unordered_map<int, std::shared_ptr<Connection>> connections_;

  int max_fd_index_ = 0;

  uint16_t port_;

  std::vector<struct pollfd> fds_;

  int max_clients_;

  std::shared_ptr<EventQueue> event_queue_;
};

}  // namespace wombat::broker::server
