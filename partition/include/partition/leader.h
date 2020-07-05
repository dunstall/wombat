// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include "glog/logging.h"
#include "log/log.h"
#include "record/request.h"
#include "tmp/server.h"
#include "util/pollable.h"

namespace wombat::broker::partition {

struct ReplicaConnection {
  ReplicaConnection() = default;

  ReplicaConnection(std::shared_ptr<Connection> _connection, uint32_t _offset)
      : connection{_connection}, offset{_offset} {}

  std::shared_ptr<Connection> connection;

  uint32_t offset;
};

class Leader : public util::Pollable {
 public:
  Leader(std::unique_ptr<Server> server, std::shared_ptr<Log> log);

  void Poll() override;

 private:
  static constexpr int kMaxReplicas = 10;
  static constexpr int kMaxSend = 1024;

  std::unordered_map<int, ReplicaConnection> replicas_;

  std::unique_ptr<Server> server_;

  std::shared_ptr<Log> log_;
};

}  // namespace wombat::broker::partition
