// Copyright 2020 Andrew Dunstall

#include "partition/consumehandler.h"

#include <cstdint>
#include <optional>
#include <memory>
#include <vector>

#include "event/event.h"
#include "frame/offset.h"
#include "frame/record.h"
#include "frame/utils.h"
#include "glog/logging.h"
#include "partition/handler.h"
#include "log/log.h"

namespace wombat::broker {

ConsumeHandler::ConsumeHandler(uint32_t id, std::shared_ptr<log::Log> log)
    : Handler(id), log_{log} {}

std::optional<Event> ConsumeHandler::Handle(const Event& evt) {
  if (!IsValidType(evt.message)) {
    LOG(ERROR) << "ConsumeHandler::Handle called with invalid type";
    return std::nullopt;
  }

  const std::optional<frame::Offset> off
      = frame::Offset::Decode(evt.message.payload());
  if (!off) {
    LOG(ERROR) << "ConsumeHandler::Handle called with invalid request";
    return std::nullopt;
  }

  const frame::Record record = Lookup(off->offset());
  const frame::Message msg{
      frame::Type::kConsumeResponse, id_, record.Encode()
  };
  return Event{msg, evt.connection};
}

bool ConsumeHandler::IsValidType(const frame::Message& msg) const {
  return msg.type() == frame::Type::kConsumeRequest;
}

frame::Record ConsumeHandler::Lookup(uint32_t offset) const {
  const std::optional<uint32_t> size = frame::DecodeU32(
      log_->Lookup(offset, sizeof(uint32_t))
  );
  if (!size) {
    // If the offset is not found return empty record.
    return frame::Record{};
  }

  const std::vector<uint8_t> record = log_->Lookup(
      offset, sizeof(uint32_t) + *size
  );
  const std::optional<frame::Record> r = frame::Record::Decode(record);
  if (r) {
    return *r;
  } else {
    // If the offset is not found return empty record.
    return frame::Record{};
  }
}

}  // namespace wombat::broker
