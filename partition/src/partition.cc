// Copyright 2020 Andrew Dunstall

#include "partition/partition.h"

#include "event/event.h"

namespace wombat::broker {

void Partition::Handle(const Event& evt) {
  events_.Push(evt);
}

}  // namespace wombat::broker
