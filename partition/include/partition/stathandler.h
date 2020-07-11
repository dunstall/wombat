// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "event/event.h"
#include "event/responder.h"
#include "frame/message.h"
#include "log/log.h"

namespace wombat::broker {

class StatHandler {
 public:
  StatHandler(std::shared_ptr<Responder> responder,
              std::shared_ptr<log::Log> log);

  // TODO(AD) Return the response so passes to responder at a higher level.
  void Handle(const Event& evt);

 private:
  bool IsValidType(const Message& msg) const;

  std::shared_ptr<Responder> responder_;

  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker
