// Copyright 2020 Andrew Dunstall

#include "partition/listener.h"

#include <memory>
#include <utility>

#include "partition/partition.h"
#include "partition/syncer.h"
#include "server/server.h"
#include "util/pollable.h"

namespace wombat::broker::partition {

Listener::Listener(Partition partition,
                   std::unique_ptr<util::Pollable> syncer,
                   std::shared_ptr<server::ResponseEventQueue> responses)
  : partition_{partition},
    syncer_{std::move(syncer)},
    responses_{responses} {}

void Listener::Poll() {
  std::optional<server::Event> event;
  while ((event = queue_->TryPop()) && event) {
    const auto resp = partition_.Handle(event->request);
    if (resp) {
      responses_->Push(server::ResponseEvent{*resp, event->connection});
    }
  }

  syncer_->Poll();
}

}  // namespace wombat::broker::partition
