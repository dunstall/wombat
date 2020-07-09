// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "partition/partition.h"
#include "partition/syncer.h"
#include "server/server.h"
#include "util/pollable.h"

namespace wombat::broker::partition {

class Listener : public util::Pollable {
 public:
  Listener(Partition partition,
           std::unique_ptr<util::Pollable> syncer,
           std::shared_ptr<server::ResponseEventQueue> responses);

  Listener(const Listener& conn) = delete;
  Listener& operator=(const Listener& conn) = delete;

  Listener(Listener&& conn) = delete;
  Listener& operator=(Listener&& conn) = delete;

  std::shared_ptr<server::EventQueue> queue() const { return queue_; }

  void Poll();

 private:
  Partition partition_;

  std::unique_ptr<util::Pollable> syncer_;

  std::shared_ptr<server::EventQueue> queue_;

  std::shared_ptr<server::ResponseEventQueue> responses_;
};

}  // namespace wombat::broker::partition
