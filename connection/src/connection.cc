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
  Read();
  if (remaining_ != 0) {
    return std::nullopt;
  }

  switch (state_) {
    case State::kHeaderPending:
      return HandleHeader();
    case State::kPayloadPending:
      return HandleMessage();
    default:
      throw ConnectionException{"invalid connection state"};
  }
}

void Connection::Send(const frame::Message& msg) {
  // TODO(AD) Temp (to keep broker in working state)
  sock_->Write(msg.Encode(), 0, msg.Encode().size());
}

std::optional<frame::Message> Connection::HandleHeader() {
  auto header = frame::MessageHeader::Decode(buf_);
  if (!header) {
    throw ConnectionException{"invalid header"};
  }

  if (header->payload_size() == 0) {
    SetHeaderPendingState();
    return frame::Message{*header, {}};
  } else {
    SetPayloadPendingState(header->payload_size());
    return std::nullopt;
  }
}

std::optional<frame::Message> Connection::HandleMessage() {
  auto msg = frame::Message::Decode(buf_);
  if (!msg) {
    throw ConnectionException{"invalid payload"};
  }

  SetHeaderPendingState();
  return *msg;
}

void Connection::Read() {
  uint32_t n = sock_->Read(&buf_, n_read_, remaining_);
  n_read_ += n;
  remaining_ -= n;
}

void Connection::SetHeaderPendingState() {
  n_read_ = 0;
  state_ = State::kHeaderPending;
  remaining_ = frame::MessageHeader::kSize;
}

void Connection::SetPayloadPendingState(uint32_t payload_size) {
  state_ = State::kPayloadPending;
  remaining_ = payload_size;
}

}  // namespace wombat::broker::connection
