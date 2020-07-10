// Copyright 2020 Andrew Dunstall

#pragma once

#include "event/event.h"

namespace wombat::broker {

class Responder {
 public:
  virtual ~Responder() {}

  // Responder(const Responder& conn) = delete;
  // Responder& operator=(const Responder& conn) = delete;

  // Responder(Responder&& conn) = delete;
  // Responder& operator=(Responder&& conn) = delete;

  virtual void Respond(const Event& evt);
};

}  // namespace wombat::broker
