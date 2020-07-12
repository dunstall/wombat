// Copyright 2020 Andrew Dunstall

#include "partition/router.h"

#include "event/event.h"

namespace wombat::broker {

Router::Router(std::shared_ptr<Responder> responder)
    : responder_{responder} {}

void Router::Route(const Event& evt) const {
  if (handlers_.find(evt.message.type()) == handlers_.end()) {
    return;
  }
  const auto resp = handlers_.at(evt.message.type())->Handle(evt);
  if (resp) {
    responder_->Respond(*resp);
  }
}

void Router::AddRoute(frame::Type type, std::unique_ptr<Handler> handler) {
  handlers_[type] = std::move(handler);
}

}  // namespace wombat::broker
