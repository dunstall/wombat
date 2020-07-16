// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "connection/event.h"
#include "util/pollable.h"

namespace wombat::broker::server {

class Responder : public util::Pollable {
 public:
  Responder();

  virtual ~Responder() {}

  Responder(const Responder& conn) = delete;
  Responder& operator=(const Responder& conn) = delete;

  Responder(Responder&& conn) = default;
  Responder& operator=(Responder&& conn) = default;

  virtual void Respond(const connection::Event& evt);

  void Poll() override;

 private:
  std::shared_ptr<connection::EventQueue> event_queue_;
};

}  // namespace wombat::broker::server
