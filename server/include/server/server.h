// Copyright 2020 Andrew Dunstall

#pragma once

#include <poll.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "glog/logging.h"
#include "log/logexception.h"
#include "record/request.h"
#include "server/connection.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

// TODO(AD) use in Leader, ConsumeServer, ProduceServer (test well)
// write R to queue

// TODO(AD) Just return Request{Type, Payload}
// then the correct handler can decode payload as required
class Server {
 public:
  explicit Server(uint16_t port, int max_clients = 1024);

  ~Server();

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = default;
  Server& operator=(Server&& conn) = default;

  std::shared_ptr<util::ThreadSafeQueue<record::Request>> queue() const {
    return queue_;
  }

  void Start();

  void Stop();

 private:
  static constexpr int kListenBacklog = 10;

  void Poll();

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

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<util::ThreadSafeQueue<record::Request>> queue_;
};

}  // namespace wombat::broker::server
