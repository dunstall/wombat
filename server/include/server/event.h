// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "record/message.h"
#include "server/connection.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

struct Event {
  Event(record::Message _message, std::shared_ptr<Connection> _connection);

  record::Message message;
  std::shared_ptr<Connection> connection;
};

using EventQueue = util::ThreadSafeQueue<Event>;

}  // namespace wombat::broker::server
