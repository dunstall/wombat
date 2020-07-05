// Copyright 2020 Andrew Dunstall

#include "partition/leader.h"

#include <memory>
#include <optional>
#include <list>

#include "record/recordrequest.h"
#include "tmp/log.h"
#include "tmp/server.h"

namespace wombat::broker::partition {

Leader::Leader(std::unique_ptr<Server> server, std::shared_ptr<Log> log)
    : server_{std::move(server)}, log_{log} {}

void Leader::Poll() {
  server_->Poll();
  std::optional<Event> evt;
  while ((evt = server_->events()->TryPop())) {
    if (evt->request.type() != record::RequestType::kReplica) {
      LOG(WARNING) << "leader received unrecognized request type";
      continue;
    }
    LOG(INFO) << "recieved replica event evt";

    std::optional<record::RecordRequest> rr = record::RecordRequest::Decode(
        evt->request.payload()
    );
    if (!rr) {
      LOG(WARNING) << "leader received invalid request record";
      continue;
    }

    replicas_[evt->connection->fd()] = ReplicaConnection{
        evt->connection, rr->offset()
    };
  }

  std::list<int> closed{};
  for (auto& e : replicas_) {
    if (!e.second.connection->closed()) {
      e.second.offset += log_->Send(e.second.offset, kMaxSend, e.first);
    } else {
      closed.push_back(e.second.connection->fd());
    }
  }

  for (int fd : closed) {
    replicas_.erase(fd);
  }
}

constexpr int Leader::kMaxReplicas;
constexpr int Leader::kMaxSend;

}  // namespace wombat::broker::partition
