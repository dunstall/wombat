// Copyright 2020 Andrew Dunstall

#include "partition/stathandler.h"

#include <memory>

#include "event/event.h"
#include "glog/logging.h"
#include "log/log.h"
#include "record/statresponse.h"

namespace wombat::broker {

StatHandler::StatHandler(std::shared_ptr<Responder> responder,
                         std::shared_ptr<log::Log> log)
    : responder_{responder}, log_{log} {}

void StatHandler::Handle(const Event& evt) {
  if (!IsValidType(evt.message)) {
    LOG(ERROR) << "StatHandler::Handle called with invalid type";
    return;
  }

  const record::StatResponse stat{log_->size()};

  // TODO(AD) Need partition ID
  const record::Message msg{
    record::MessageType::kStatResponse, 0, stat.Encode()
  };

  responder_->Respond({msg, evt.connection});
}

bool StatHandler::IsValidType(const record::Message& msg) const {
  return msg.type() == record::MessageType::kStatRequest;
}

}  // namespace wombat::broker
