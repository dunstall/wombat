// Copyright 2020 Andrew Dunstall

#include "event/event.h"

#include <memory>

#include "frame/message.h"

namespace wombat::broker {

Event::Event(frame::Message _message, std::shared_ptr<Connection> _connection)
    : message{_message}, connection{_connection} {}

bool Event::operator==(const Event& evt) const {
  return message == evt.message && connection == evt.connection;
}

bool Event::operator!=(const Event& evt) const { return !(*this == evt); }

}  // namespace wombat::broker
