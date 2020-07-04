// Copyright 2020 Andrew Dunstall

#pragma once

#include <optional>
#include <memory>
#include <vector>

#include "glog/logging.h"
#include "log/log.h"
#include "record/consumerequest.h"
#include "record/record.h"
#include "record/request.h"
#include "record/response.h"

namespace wombat::broker::partition {

template<class S>
class Partition {
 public:
  explicit Partition(std::shared_ptr<log::Log<S>> log) : log_{log} {}

  std::optional<record::Response> Handle(const record::Request& request) {
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

 private:
  void Produce(const record::Request& request) {
    // Decode the record to ensure only valid records are appended.
    std::optional<record::Record> record
        = record::Record::Decode(request.payload());
    if (record) {
      log_->Append(record->Encode());
    } else {
      LOG(WARNING) << "received invalid produce record";
    }
  }

  std::optional<record::Response> Consume(const record::Request& request) {
    std::optional<record::ConsumeRequest> cr
        = record::ConsumeRequest::Decode(request.payload());
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

  std::shared_ptr<log::Log<S>> log_;
};

}  // namespace wombat::broker::partition
