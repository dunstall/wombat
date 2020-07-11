// Copyright 2020 Andrew Dunstall

#include "partition/consumehandler.h"

#include <memory>

#include "event/event.h"
#include "frame/record.h"
#include "frame/recordrequest.h"
#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker {

ConsumeHandler::ConsumeHandler(std::shared_ptr<Responder> responder,
                               std::shared_ptr<log::Log> log)
    : responder_{responder}, log_{log} {}

void ConsumeHandler::Handle(const Event& evt) {
  if (!IsValidType(evt.message)) {
    LOG(ERROR) << "ConsumeHandler::Handle called with invalid type";
    return;
  }

  const std::optional<record::RecordRequest> rr
      = record::RecordRequest::Decode(evt.message.payload());
  if (!rr) {
    LOG(ERROR) << "ConsumeHandler::Handle called with invalid request";
    return;
  }

  const std::optional<record::Record> record = Lookup(rr->offset());
  if (!record) {
    // If the offset is not found return empty record.
    const record::Message msg{
      record::MessageType::kConsumeResponse, 0, record::Record{}.Encode()
    };
    responder_->Respond({msg, evt.connection});
    return;
  }

  // TODO(AD) Need partition ID
  const record::Message msg{
    record::MessageType::kConsumeResponse, 0, record->Encode()
  };

  responder_->Respond({msg, evt.connection});
}

bool ConsumeHandler::IsValidType(const record::Message& msg) const {
  return msg.type() == record::MessageType::kConsumeRequest;
}

std::optional<record::Record> ConsumeHandler::Lookup(uint32_t offset) const {
  const std::optional<uint32_t> size = record::DecodeU32(
      log_->Lookup(offset, sizeof(uint32_t))
  );
  if (!size) {
    return std::nullopt;
  }

  const std::vector<uint8_t> record = log_->Lookup(
      offset, sizeof(uint32_t) + *size
  );
  return record::Record::Decode(record);
}

}  // namespace wombat::broker
