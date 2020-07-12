// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "event/event.h"
#include "frame/message.h"
#include "log/log.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class StatHandler : public Handler {
 public:
  StatHandler(uint32_t id, std::shared_ptr<log::Log> log);

  ~StatHandler() override {}

  std::optional<Event> Handle(const Event& evt) override;

 private:
  bool IsValidType(const frame::Message& msg) const;

  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker::partition
