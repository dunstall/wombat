// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "event/event.h"
#include "frame/message.h"
#include "frame/record.h"
#include "log/log.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class ConsumeHandler : public Handler {
 public:
  ConsumeHandler(uint32_t id, std::shared_ptr<log::Log> log);

  ~ConsumeHandler() override {}

  std::optional<Event> Handle(const Event& evt);

 private:
  bool IsValidType(const frame::Message& msg) const;

  frame::Record Lookup(uint32_t offset) const;

  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker::partition
