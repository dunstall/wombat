// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "connection/socket.h"
#include "frame/message.h"

namespace wombat::broker::connection {

class Connection {
 public:
  explicit Connection(std::unique_ptr<Socket> sock);

  virtual ~Connection() {}

  // Reads from the connection. If a full message has been received it is
  // returned otherwise returns nullopt.
  //
  // If there is an error with the underlying socket or if the connection
  // receives an invalid message a ConnectionException is thrown. The
  // connection should be closed.
  virtual std::optional<frame::Message> Receive();

  // TODO(AD) On send register with a singleton thread that keeps polling
  // until the full outgoing buffer is emptied - just register socket?
  virtual void Send(const frame::Message& msg);

 private:
  enum class State { kHeaderPending, kPayloadPending };

  static constexpr size_t kBufSize = 1024;

  std::optional<frame::Message> HandleHeader();

  std::optional<frame::Message> HandleMessage();

  void Read();

  void SetHeaderPendingState();

  void SetPayloadPendingState(uint32_t payload_size);

  std::vector<uint8_t> buf_;

  std::unique_ptr<Socket> sock_;

  State state_;

  uint32_t n_read_;
  uint32_t remaining_;
};

}  // namespace wombat::broker::connection
