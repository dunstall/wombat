// Copyright 2020 Andrew Dunstall

#include "connection/event.h"

#include <memory>

#include "connection/connection.h"
#include "frame/message.h"

namespace wombat::broker::connection {

Event::Event(frame::Message _message, std::shared_ptr<Connection> _connection)
    : message{_message}, connection{_connection} {}

bool Event::operator==(const Event& evt) const {
  return message == evt.message && connection == evt.connection;
}

bool Event::operator!=(const Event& evt) const { return !(*this == evt); }

}  // namespace wombat::broker::connection
