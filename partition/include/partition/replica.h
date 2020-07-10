// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "event/event.h"
#include "log/log.h"
#include "partition/partition.h"

namespace wombat::broker {

class Replica : public Partition {
 public:
  Replica(std::shared_ptr<Responder> responder,
          std::shared_ptr<log::Log> log) : Partition{0} {}

  void Handle(const Event& evt) override {}
};

}  // namespace wombat::broker
