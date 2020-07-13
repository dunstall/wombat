// Copyright 2020 Andrew Dunstall

#include "partition/producehandler.h"

#include <memory>
#include <optional>

#include "event/event.h"
#include "frame/record.h"
#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker::partition {

ProduceHandler::ProduceHandler(uint32_t id, std::shared_ptr<log::Log> log)
    : Handler(id), log_{log} {}

std::optional<Event> ProduceHandler::Handle(const Event& evt) {
  if (evt.message.type() != frame::Type::kProduceRequest) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid type";
    return std::nullopt;
  }

  const std::optional<frame::Record> record =
      frame::Record::Decode(evt.message.payload());
  if (!record) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid record";
    return std::nullopt;
  }

  log_->Append(record->Encode());
  return std::nullopt;
}

}  // namespace wombat::broker::partition
