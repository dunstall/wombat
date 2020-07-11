// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "frame/message.h"
#include "log/log.h"

namespace wombat::broker {

class ProduceHandler {
 public:
  explicit ProduceHandler(std::shared_ptr<log::Log> log);

  void Handle(const Message& msg);

 private:
  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker
