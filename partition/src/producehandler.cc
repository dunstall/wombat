// Copyright 2020 Andrew Dunstall

#include "partition/producehandler.h"

#include <memory>
#include <optional>

#include "frame/message.h"
#include "frame/record.h"
#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker {

ProduceHandler::ProduceHandler(std::shared_ptr<log::Log> log) : log_{log} {}

void ProduceHandler::Handle(const Message& msg) {
  if (msg.type() != MessageType::kProduceRequest) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid type";
    return;
  }

  const std::optional<Record> record
      = Record::Decode(msg.payload());
  if (!record) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid record";
    return;
  }

  log_->Append(record->Encode());
}

}  // namespace wombat::broker
