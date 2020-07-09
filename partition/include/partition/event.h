// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "record/message.h"
#include "util/threadsafequeue.h"

namespace wombat::broker {

class Connection {
 public:
  virtual ~Connection() {}
};

struct Event {
  Event(record::Message _message, std::shared_ptr<Connection> _connection);

  bool operator==(const Event& evt) const;

  bool operator!=(const Event& evt) const;

  record::Message message;
  std::shared_ptr<Connection> connection;
};

using EventQueue = util::ThreadSafeQueue<Event>;

}  // namespace wombat::broker
