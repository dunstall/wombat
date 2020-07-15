// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "connection/connection.h"
#include "frame/message.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::connection {

struct Event {
  Event(frame::Message _message, std::shared_ptr<Connection> _connection);

  bool operator==(const Event& evt) const;

  bool operator!=(const Event& evt) const;

  frame::Message message;
  std::shared_ptr<Connection> connection;
};

using EventQueue = util::ThreadSafeQueue<Event>;

}  // namespace wombat::broker::connection
