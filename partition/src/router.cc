// Copyright 2020 Andrew Dunstall

#include "partition/router.h"

#include "event/event.h"

namespace wombat::broker {

void Router::Route(const Event& evt) const {
}

void Router::AddRoute(frame::Type type, std::unique_ptr<Handler> handler) {
}

}  // namespace wombat::broker
