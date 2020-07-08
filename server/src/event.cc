// Copyright 2020 Andrew Dunstall

#include <memory>

#include "record/message.h"
#include "server/event.h"
#include "server/connection.h"

namespace wombat::broker::server {

Event::Event(record::Message _message, std::shared_ptr<Connection> _connection)
    : message{_message}, connection{_connection} {
}

bool Event::operator==(const Event& evt) const {
  return message == evt.message && connection == evt.connection;
}

bool Event::operator!=(const Event& evt) const {
  return !(*this == evt);
}

}  // namespace wombat::broker::server
