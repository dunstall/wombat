// Copyright 2020 Andrew Dunstall

#pragma once

#include "connection/event.h"

namespace wombat::broker {

// TODO(AD) Implement - move to own package and remove the event package
class Responder {
 public:
  virtual ~Responder() {}

  // Responder(const Responder& conn) = delete;
  // Responder& operator=(const Responder& conn) = delete;

  // Responder(Responder&& conn) = delete;
  // Responder& operator=(Responder&& conn) = delete;

  virtual void Respond(const connection::Event& evt);
};

}  // namespace wombat::broker
