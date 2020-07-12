// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>

#include "event/event.h"

namespace wombat::broker {

class Handler {
 public:
  explicit Handler(uint32_t id);

  virtual ~Handler() {}

  virtual std::optional<Event> Handle(const Event& evt) = 0;

 protected:
  uint32_t id_;
};

}  // namespace wombat::broker
