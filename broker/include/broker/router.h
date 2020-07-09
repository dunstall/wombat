// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <unordered_map>

#include "log/log.h"
#include "server/event.h"

namespace wombat::broker {

class Partition {
 public:
  explicit Partition(uint32_t id) : id_{id} {}

  virtual ~Partition() {}

  uint32_t id() const { return id_; }

  virtual void Handle(const server::Event& evt) = 0;

 protected:
  uint32_t id_;
};

class Handler {
 public:
  virtual void Handle(const server::Event& evt) = 0;
};

class Responder : public Handler {
 public:
  void Handle(const server::Event& evt) {}
};

class LeaderPartition : public Partition {
 public:
  LeaderPartition(std::shared_ptr<Handler> handler,
                  std::shared_ptr<log::Log> log) : Partition{0} {}

  void Handle(const server::Event& evt) override {}
};

class ReplicaPartition : public Partition {
 public:
  ReplicaPartition(std::shared_ptr<Handler> handler,
                   std::shared_ptr<log::Log> log) : Partition{0} {}

  void Handle(const server::Event& evt) override {}
};

class Router {
 public:
  bool Route(const server::Event& evt);

  void AddPartition(std::unique_ptr<Partition> partition);

 private:
  std::unordered_map<uint32_t, std::unique_ptr<Partition>> partitions_;
};

}  // namespace wombat::broker
