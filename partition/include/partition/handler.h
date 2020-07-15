// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <optional>

#include "connection/event.h"

namespace wombat::broker::partition {

class Handler {
 public:
  explicit Handler(uint32_t id);

  virtual ~Handler() {}

  virtual std::optional<connection::Event> Handle(
      const connection::Event& evt) = 0;

 protected:
  uint32_t id_;
};

}  // namespace wombat::broker::partition
