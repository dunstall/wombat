#include <chrono>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>
#include "log/framing.h"
#include "log/leader.h"
#include "log/log.h"
#include "log/replica.h"
#include "log/inmemorysegment.h"
#include "log/systemsegment.h"
#include "log/tempdir.h"

enum class Type {
  kLeader,
  kReplica
};

Type ParseType(int argc, char** argv) {
  if (strcmp(argv[1], "leader") == 0) {
    return Type::kLeader;
  } else if (strcmp(argv[1], "replica") == 0) {
    return Type::kReplica;
  }

  std::cout << "unknown type" << std::endl;
  std::exit(1);
}

std::vector<uint16_t> ParsePorts(int argc, char** argv) {
  std::vector<uint16_t> ports{};
  for (int i = 2; i != argc; ++i) {
    ports.push_back(std::stoi(std::string(argv[i])));
  }
  return ports;
}

void RunLeader(const std::vector<uint16_t>& ports) {
  std::cout << "running as leader - replicas at ";
  for (auto p : ports) {
    std::cout << p << ", ";
  }
  std::cout << std::endl;

  wombat::log::TempDir dir{};
  wombat::log::Log<wombat::log::InMemorySegment> log{dir.path(), 128'000'000};

  wombat::log::Leader<wombat::log::InMemorySegment> leader{log};

  wombat::log::Framing<wombat::log::InMemorySegment> framing{log};

  for (std::string line; std::getline(std::cin, line);) {
    std::cout << "Append " << line << std::endl;
    framing.Append(std::vector<uint8_t>(line.begin(), line.end()));
  }
}

void RunReplica(const std::vector<uint16_t>& ports) {
    wombat::log::TempDir dir{};
    wombat::log::Replica<wombat::log::InMemorySegment> rep{
        wombat::log::Log<wombat::log::InMemorySegment>{dir.path(), 128'000'000},
        {"127.0.0.1", 3110}
    };
    std::cout << "running as replica on port " << ports[0] << std::endl;
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc < 3) {
    std::cout << "USAGE: ./log TYPE PORTS" << std::endl;
    std::exit(1);
  }

  switch (ParseType(argc, argv)) {
  case Type::kLeader:
    RunLeader(ParsePorts(argc, argv));
    break;
  case Type::kReplica:
    RunReplica(ParsePorts(argc, argv));
    break;
  }
}
