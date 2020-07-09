// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "record/message.h"

namespace wombat::broker {

class ProduceHandler {
 public:
  ProduceHandler(std::shared_ptr<log::Log> log);

  void Handle(const record::Message& msg);

 private:
  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker
