// Copyright 2020 Andrew Dunstall

#include "partition/stathandler.h"

#include <memory>

#include "connection/event.h"
#include "frame/offset.h"
#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker::partition {

StatHandler::StatHandler(uint32_t id, std::shared_ptr<log::Log> log)
    : Handler(id), log_{log} {}

std::optional<connection::Event> StatHandler::Handle(
    const connection::Event& evt) {
  if (!IsValidType(evt.message)) {
    LOG(ERROR) << "StatHandler::Handle called with invalid type";
    return std::nullopt;
  }

  const frame::Offset stat{log_->size()};
  const frame::Message msg{frame::Type::kStatResponse, id_, stat.Encode()};
  return connection::Event{msg, evt.connection};
}

bool StatHandler::IsValidType(const frame::Message& msg) const {
  return msg.type() == frame::Type::kStatRequest;
}

}  // namespace wombat::broker::partition
