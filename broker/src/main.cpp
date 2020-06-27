#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <glog/logging.h>
#include "log/log.h"
#include "log/systemsegment.h"
#include "log/tempdir.h"
#include "partition/leader.h"
#include "partition/replica.h"

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

    wombat::broker::TempDir dir{};
    std::shared_ptr<wombat::broker::Log<wombat::broker::SystemSegment>> log
        = std::make_shared<wombat::broker::Log<wombat::broker::SystemSegment>>(dir.path(), 128'000'000);
    wombat::broker::Leader<wombat::broker::SystemSegment> leader{log, 3110};

    while (true) {
      leader.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // TODO(AD) run server
  } else if (std::string(argv[1]) == "replica" && argc == 3) {
    std::cout << "replica with leader address " << argv[2] << std::endl;
     wombat::broker::TempDir dir{};
    std::shared_ptr<wombat::broker::Log<wombat::broker::SystemSegment>> log
        = std::make_shared<wombat::broker::Log<wombat::broker::SystemSegment>>(dir.path(), 128'000'000);
    wombat::broker::Replica<wombat::broker::SystemSegment> replica{log, {"127.0.0.1", 3110}};

    while (true) {
      replica.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // TODO(AD) run server
  } else {
    PrintUsage();
    std::exit(EXIT_FAILURE);
  }
}
