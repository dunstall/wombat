// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "record/request.h"
#include "server/event.h"

namespace wombat::broker {

class Connection {
 public:
  explicit Connection(int fd) : fd_{fd} {}

  int fd() const { return fd_; }

  bool closed() const { return closed_; }

  void close() { closed_ = true; }

 private:
  int fd_;

  bool closed_ = false;
};

struct Event {
  Event(record::Request _request, std::shared_ptr<Connection> _connection)
      : request{_request}, connection{_connection} {}

  record::Request request;
  std::shared_ptr<Connection> connection;
};

using EventQueue = util::ThreadSafeQueue<Event>;

class Server {
 public:
  virtual ~Server() {}

  virtual void Poll() = 0;

  std::shared_ptr<EventQueue> events() const { return event_queue_; }

 protected:
  std::shared_ptr<EventQueue> event_queue_ = std::make_shared<EventQueue>();
};

}  // namespace wombat::broker
