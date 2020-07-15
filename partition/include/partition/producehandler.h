// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <optional>

#include "connection/event.h"
#include "log/log.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class ProduceHandler : public Handler {
 public:
  ProduceHandler(uint32_t id, std::shared_ptr<log::Log> log);

  ~ProduceHandler() override {}

  std::optional<connection::Event> Handle(
      const connection::Event& evt) override;

 private:
  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker::partition
