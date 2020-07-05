// Copyright 2020 Andrew Dunstall

#include "partition/partition.h"

#include <optional>
#include <memory>
#include <vector>

#include "glog/logging.h"
#include "record/record.h"
#include "record/recordrequest.h"
#include "record/request.h"
#include "record/response.h"
#include "tmp/log.h"

namespace wombat::broker::partition {

Partition::Partition(std::shared_ptr<Log> log) : log_{log} {}

std::optional<record::Response> Partition::Handle(
    const record::Request& request) {
  switch (request.type()) {
    case record::RequestType::kProduce:
      Produce(request);
      break;
    case record::RequestType::kConsume:
      return Consume(request);
    default:
      LOG(WARNING) << "unrecognized request type";
  }
  return std::nullopt;
}

void Partition::Produce(const record::Request& request) {
  // Decode the record to ensure only valid records are appended.
  std::optional<record::Record> record
      = record::Record::Decode(request.payload());
  if (record) {
    log_->Append(record->Encode());
  } else {
    LOG(WARNING) << "received invalid produce record";
  }
}

std::optional<record::Response> Partition::Consume(
    const record::Request& request) {
  std::optional<record::RecordRequest> cr
      = record::RecordRequest::Decode(request.payload());
  if (!cr) {
    // If invalid request return nothing.
    return std::nullopt;
  }

  std::optional<uint32_t> size = record::DecodeU32(
      log_->Lookup(cr->offset(), sizeof(uint32_t))
  );
  if (!size) {
    // If the offset is not found return empty record.
    return record::Response{
      record::ResponseType::kConsume, record::Record{}.Encode()
    };
  }

  // Since the records must be valid this will return size.
  const std::vector<uint8_t> data = log_->Lookup(
      cr->offset(), sizeof(uint32_t) + *size
  );
  return record::Response{record::ResponseType::kConsume, data};
}

}  // namespace wombat::broker::partition
