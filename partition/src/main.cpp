#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "log/leader.h"
#include "log/log.h"
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

  wombat::log::TempDir dir{};
  wombat::log::Leader<wombat::log::SystemSegment> leader{
      wombat::log::Log<wombat::log::SystemSegment>{dir.path(), 128'000'000}
  };
  std::cout << std::endl;
  for (std::string line; std::getline(std::cin, line);) {
    std::cout << "Append " << line << std::endl;
    leader.Append(std::vector<uint8_t>(line.begin(), line.end()));
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "USAGE: ./log TYPE PORTS" << std::endl;
    std::exit(1);
  }

  switch (ParseType(argc, argv)) {
  case Type::kLeader:
    RunLeader(ParsePorts(argc, argv));
    break;
  case Type::kReplica:
    std::cout << "running as replica on port " << ParsePorts(argc, argv)[0] << std::endl;
    break;
  }
}
