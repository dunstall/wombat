// Copyright 2020 Andrew Dunstall

#pragma once

#include <poll.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "connection/connection.h"
#include "connection/event.h"
#include "util/pollable.h"

namespace wombat::broker::server {

// Listener handles reading requests from connections to clients. This does
// not write to the clients (thats left to responder).
class Listener : public util::Pollable {
 public:
  explicit Listener(uint16_t port, int max_clients = 1024);

  ~Listener() override {}

  Listener(const Listener& conn) = delete;
  Listener& operator=(const Listener& conn) = delete;

  Listener(Listener&& conn) = delete;
  Listener& operator=(Listener&& conn) = delete;

  std::shared_ptr<connection::EventQueue> events() const {
    return event_queue_;
  }

  void Poll() override;

 private:
  static constexpr int kListenBacklog = 10;
  static constexpr int kPollTimeoutMS = 1000;

  void Listen();

  void Accept();

  void Read(int i);

  bool PendingConnection() const;

  bool PendingRead(int i) const;

  int listenfd_;

  std::unordered_map<int, std::shared_ptr<connection::Connection>> connections_;

  int max_fd_index_ = 0;

  uint16_t port_;

  std::vector<struct pollfd> fds_;

  int max_clients_;

  std::shared_ptr<connection::EventQueue> event_queue_;
};

}  // namespace wombat::broker::server
