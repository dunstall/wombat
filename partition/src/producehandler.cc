// Copyright 2020 Andrew Dunstall

#include "partition/producehandler.h"

#include <memory>
#include <optional>

#include "glog/logging.h"
#include "log/log.h"
#include "record/message.h"
#include "record/record.h"

namespace wombat::broker {

ProduceHandler::ProduceHandler(std::shared_ptr<log::Log> log) : log_{log} {}

void ProduceHandler::Handle(const record::Message& msg) {
  if (msg.type() != record::MessageType::kProduceRequest) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid type";
    return;
  }

  const std::optional<record::Record> record
      = record::Record::Decode(msg.payload());
  if (!record) {
    LOG(ERROR) << "ProduceHandler::Handle called with invalid record";
    return;
  }

  log_->Append(record->Encode());
}

}  // namespace wombat::broker
