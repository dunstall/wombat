// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <optional>

#include "event/event.h"
#include "log/log.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class ProduceHandler : public Handler {
 public:
  ProduceHandler(uint32_t id, std::shared_ptr<log::Log> log);

  ~ProduceHandler() override {}

  std::optional<Event> Handle(const Event& evt) override;

 private:
  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker::partition
