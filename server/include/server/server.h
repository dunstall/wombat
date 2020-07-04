// Copyright 2020 Andrew Dunstall

#pragma once

#include <poll.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "record/response.h"
#include "record/request.h"
#include "server/connection.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

struct Event {
  Event(record::Request _request, std::shared_ptr<Connection> _connection);

  record::Request request;
  std::shared_ptr<Connection> connection;
};

using EventQueue = util::ThreadSafeQueue<Event>;

struct ResponseEvent {
  ResponseEvent(record::Response _response, std::shared_ptr<Connection> _connection);

  record::Response response;
  std::shared_ptr<Connection> connection;
};

using ReponseEventQueue = util::ThreadSafeQueue<ResponseEvent>;


// Server handles reading requests from connections to clients. This does
// not write to the clients (thats left to responder).
class Server {
 public:
  explicit Server(uint16_t port, int max_clients = 1024);

  ~Server();

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = default;
  Server& operator=(Server&& conn) = default;

  std::shared_ptr<EventQueue> events() const { return event_queue_; }

 private:
  static constexpr int kListenBacklog = 10;
  static constexpr int kPollTimeoutMS = 1000;

  void Start();

  void Stop();

  void Poll();

  void Listen();

  void Accept();

  void Read(int i);

  bool PendingConnection() const;

  bool PendingRead(int i) const;

  bool PendingWrite(int i) const;

  int listenfd_;

  std::unordered_map<int, std::shared_ptr<Connection>> connections_;

  int max_fd_index_ = 0;

  std::thread thread_;
  std::atomic_bool running_;

  uint16_t port_;

  std::vector<struct pollfd> fds_;

  int max_clients_;

  std::shared_ptr<EventQueue> event_queue_;
};

}  // namespace wombat::broker::server
