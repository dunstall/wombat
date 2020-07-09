// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "partition/event.h"
#include "record/message.h"
#include "record/record.h"

namespace wombat::broker {

class Responder {
 public:
  virtual void Respond(const Event& evt) = 0;
};

class ConsumeHandler {
 public:
  ConsumeHandler(std::shared_ptr<Responder> responder,
                 std::shared_ptr<log::Log> log);

  // TODO(AD) Return the response so passes to responder at a higher level.
  void Handle(const Event& evt);

 private:
  bool IsValidType(const record::Message& msg) const;

  std::optional<record::Record> Lookup(uint32_t offset) const;

  std::shared_ptr<Responder> responder_;

  std::shared_ptr<log::Log> log_;
};

}  // namespace wombat::broker
