#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>

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

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "USAGE: ./log <replica | leader > <ports>" << std::endl;
    std::exit(1);
  }

  switch (ParseType(argc, argv)) {
  case Type::kLeader:
    std::cout << "running as leader - replicas at ";
    for (auto p : ParsePorts(argc, argv)) {
      std::cout << p << ", ";
    }
    std::cout << std::endl;
    break;
  case Type::kReplica:
    std::cout << "running as replica on port " << ParsePorts(argc, argv)[0] << std::endl;
    break;
  }
}
