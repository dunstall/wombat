// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "partition/event.h"

namespace wombat::broker {

class Partition {
 public:
  explicit Partition(uint32_t id) : id_{id} {}

  virtual ~Partition() {}

  uint32_t id() const { return id_; }

  virtual void Handle(const Event& evt);

 protected:
  EventQueue events_;

  uint32_t id_;
};

}  // namespace wombat::broker
