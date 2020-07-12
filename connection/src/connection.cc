// Copyright 2020 Andrew Dunstall

#include "connection/connection.h"

#include <optional>
#include <memory>

#include "connection/socket.h"
#include "frame/message.h"

namespace wombat::broker::connection {

Connection::Connection(std::unique_ptr<Socket> sock) {}

std::optional<frame::Message> Connection::Receive() {
  return std::nullopt;
}

void Connection::Send(const frame::Message& msg) {}

}  // namespace wombat::broker::connection
