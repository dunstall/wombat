// Copyright 2020 Andrew Dunstall

#pragma once

#include <optional>
#include <memory>

#include "connection/socket.h"
#include "frame/message.h"

namespace wombat::broker::connection {

class Connection {
 public:
  explicit Connection(std::unique_ptr<Socket> sock);

  virtual ~Connection() {}

  virtual std::optional<frame::Message> Receive();

  // TODO(AD) On send register with a singleton thread that keeps polling
  // until the full outgoing buffer is emptied - just register socket?
  virtual void Send(const frame::Message& msg);
};

}  // namespace wombat::broker::connection
