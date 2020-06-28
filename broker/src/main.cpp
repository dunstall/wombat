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
#include "record/consumerecord.h"
#include "record/producerecord.h"
#include "server/server.h"

namespace wombat::broker {

void RunLeader() {
  log::TempDir dir{};
  std::shared_ptr<log::Log<log::SystemSegment>> log
      = std::make_shared<log::Log<log::SystemSegment>>(dir.path(), 128'000'000);

  server::Server<record::ProduceRecord> server{3111};
  server.Start();

  partition::PartitionLeader partition{log, 3110};
  partition.Start();

  // TODO(AD) Broker will handle routing requests to the correct partition.
  while (true) {
    partition.queue()->Push(server.queue()->WaitAndPop());
  }
}

void RunReplica() {
  log::TempDir dir{};
  std::shared_ptr<log::Log<log::SystemSegment>> log
      = std::make_shared<log::Log<log::SystemSegment>>(dir.path(), 128'000'000);
  partition::Replica<log::SystemSegment> replica{log, {"127.0.0.1", 3110}};

  server::Server<record::ConsumeRecord> server{3112};

  while (true) {
    std::optional<record::ConsumeRecord> record;
    while ((record = server.queue()->TryPop()) && record) {
      // TODO(AD) handle - send to partition queue
    }

    replica.Poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    wombat::broker::RunLeader();
  } else if (std::string(argv[1]) == "replica" && argc == 3) {
    std::cout << "replica with leader address " << argv[2] << std::endl;
    wombat::broker::RunReplica();
  } else {
    PrintUsage();
    std::exit(EXIT_FAILURE);
  }
}
