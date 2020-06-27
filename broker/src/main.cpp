// Copyright 2020 Andrew Dunstall

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "log/log.h"
#include "log/systemsegment.h"
#include "log/tempdir.h"
#include "partition/leader.h"
#include "partition/replica.h"
#include "record/consumerecord.h"
#include "record/producerecord.h"
#include "consumeserver/server.h"
#include "produceserver/server.h"

namespace wombat::broker {

void RunLeader() {
  log::TempDir dir{};
  std::shared_ptr<log::Log<log::SystemSegment>> log
      = std::make_shared<log::Log<log::SystemSegment>>(dir.path(), 128'000'000);
  partition::Leader<log::SystemSegment> leader{log, 3110};

  produceserver::Server server{3111};

  while (true) {
    std::vector<record::ProduceRecord> requests = server.Poll();
    for (const auto r : requests) {
      log->Append(r.Encode());
    }

    leader.Poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void RunReplica() {
  log::TempDir dir{};
  std::shared_ptr<log::Log<log::SystemSegment>> log
      = std::make_shared<log::Log<log::SystemSegment>>(dir.path(), 128'000'000);
  partition::Replica<log::SystemSegment> replica{log, {"127.0.0.1", 3110}};

  consumeserver::Server server{3112};

  while (true) {
    std::vector<record::ConsumeRecord> requests = server.Poll();
    // TODO(AD) handle consuume requests

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
