// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

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
  ResponseEvent(record::Response _response,
                std::shared_ptr<Connection> _connection);

  record::Response response;
  std::shared_ptr<Connection> connection;
};

using ResponseEventQueue = util::ThreadSafeQueue<ResponseEvent>;

}  // namespace wombat::broker::server
