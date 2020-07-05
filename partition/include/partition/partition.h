// Copyright 2020 Andrew Dunstall

#pragma once

#include <optional>
#include <memory>
#include <vector>

#include "record/request.h"
#include "record/response.h"
#include "tmp/log.h"

namespace wombat::broker::partition {

class Partition {
 public:
  explicit Partition(std::shared_ptr<Log> log);

  std::optional<record::Response> Handle(const record::Request& request);

 private:
  void Produce(const record::Request& request);

  std::optional<record::Response> Consume(const record::Request& request);

  std::shared_ptr<Log> log_;
};

}  // namespace wombat::broker::partition
