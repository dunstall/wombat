// Copyright 2020 Andrew Dunstall

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "log/log.h"
#include "log/systemsegment.h"
#include "log/tempdir.h"
#include "partition/leader.h"
#include "partition/partition.h"
#include "partition/replica.h"
#include "partition/syncer.h"
#include "record/consumerecord.h"
#include "record/producerecord.h"
#include "server/server.h"

namespace wombat::broker {

enum class Type {
  kLeader,
  kReplica
};

void Run(Type type) {
  log::TempDir dir{};
  std::shared_ptr<log::Log<log::SystemSegment>> log
      = std::make_shared<log::Log<log::SystemSegment>>(dir.path(), 128'000'000);

  server::Server server{3111};

  std::unique_ptr<partition::Syncer<log::SystemSegment>> syncer;
  switch(type) {
    case Type::kLeader:
     syncer = std::make_unique<partition::Leader<log::SystemSegment>>(
         log, 3110
     );
     break;
    case Type::kReplica:
     syncer = std::make_unique<partition::Replica<log::SystemSegment>>(
         log, partition::LeaderAddress{"127.0.0.1", 3110}
     );
     break;
    default:
     std::exit(1);
     break;
  }

  partition::Partition partition{log, 3110, std::move(syncer)};
  partition.Start();

  // TODO(AD) Broker will handle routing requests to the correct partition.
  while (true) {
    partition.queue()->Push(server.events()->WaitAndPop().request);
  }
}

}  // namespace wombat::broker

void PrintUsage() {
  std::cout << "USAGE:" << std::endl;
  std::cout << "\t./broker leader" << std::endl;
  std::cout << "\t\tor" << std::endl;
  std::cout << "\t./broker replica <leader IP>" << std::endl;
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc < 2) {
    PrintUsage();
    std::exit(EXIT_FAILURE);
  }

  if (std::string(argv[1]) == "leader") {
    std::cout << "leader" << std::endl;
    wombat::broker::Run(wombat::broker::Type::kLeader);
  } else if (std::string(argv[1]) == "replica" && argc == 3) {
    std::cout << "replica with leader address " << argv[2] << std::endl;
    wombat::broker::Run(wombat::broker::Type::kReplica);
  } else {
    PrintUsage();
    std::exit(EXIT_FAILURE);
  }
}
