// Copyright 2020 Andrew Dunstall

#pragma once

#include <optional>
#include <memory>
#include <vector>

#include "record/message.h"
#include "log/log.h"

namespace wombat::broker::partition {

class Partition {
 public:
  explicit Partition(std::shared_ptr<Log> log);

  std::optional<record::Message> Handle(const record::Message& message);

 private:
  void Produce(const record::Message& message);

  std::optional<record::Message> Consume(const record::Message& message);

  std::shared_ptr<Log> log_;
};

}  // namespace wombat::broker::partition
