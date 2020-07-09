// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "server/event.h"

namespace wombat::broker {

class Partition {
 public:
  explicit Partition(uint32_t id) : id_{id} {}

  virtual ~Partition() {}

  uint32_t id() const { return id_; }

  virtual void Handle(const server::Event& evt) = 0;

 protected:
  uint32_t id_;
};

class Handler {
 public:
  virtual void Handle(const server::Event& evt) = 0;
};

class Responder : public Handler {
 public:
  void Handle(const server::Event& evt) {}
};

}  // namespace wombat::broker
