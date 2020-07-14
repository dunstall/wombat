// Copyright 2020 Andrew Dunstall

#include "connection/connection.h"

#include <memory>
#include <optional>

#include "connection/connectionexception.h"
#include "connection/socket.h"
#include "frame/message.h"

namespace wombat::broker::connection {

Connection::Connection(std::unique_ptr<Socket> sock)
    : buf_(kBufSize),
      sock_{std::move(sock)},
      state_{State::kHeaderPending},
      n_read_{0},
      remaining_{frame::MessageHeader::kSize} {}

std::optional<frame::Message> Connection::Receive() {
  uint32_t n = sock_->Read(&buf_, n_read_, remaining_);
  n_read_ += n;
  remaining_ -= n;

  if (remaining_ == 0) {
    if (state_ == State::kHeaderPending) {
      auto header = frame::MessageHeader::Decode(buf_);
      if (!header) {
        throw ConnectionException{"invalid header"};
      }

      if (header->payload_size() == 0) {
        n_read_ = 0;
        remaining_ = frame::MessageHeader::kSize;
        return frame::Message{*header, {}};
      }

      state_ = State::kPayloadPending;
      remaining_ = header->payload_size();
    } else if (state_ == State::kPayloadPending) {
      auto msg = frame::Message::Decode(buf_);
      if (!msg) {
        throw ConnectionException{"invalid payload"};
      }

      state_ = State::kHeaderPending;
      n_read_ = 0;
      remaining_ = frame::MessageHeader::kSize;

      return *msg;
    }
  }

  return std::nullopt;
}

void Connection::Send(const frame::Message& msg) {}

}  // namespace wombat::broker::connection
