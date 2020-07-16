// Copyright 2020 Andrew Dunstall

#include <optional>

#include "connection/event.h"
#include "server/responder.h"

namespace wombat::broker::server {

Responder::Responder()
    : event_queue_{std::make_shared<connection::EventQueue>()} {}

void Responder::Respond(const connection::Event& evt) {
  event_queue_->Push(evt);
}

void Responder::Poll() {
  std::optional<connection::Event> evt;
  while ((evt = event_queue_->TryPop())) {
    // TODO(AD) Keep polling until all sent
    evt->connection->Send(evt->message);
  }
}

}  // namespace wombat::broker::server
