// Copyright 2020 Andrew Dunstall

#include "broker/conf.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "glog/logging.h"

namespace wombat::broker {

std::vector<std::string> Split(const std::string& s, char delimiter);

Conf::Conf(std::list<PartitionConf> partitions) : partitions_{partitions} {}

std::optional<Conf> Conf::Parse(const std::string& s) {
  std::list<PartitionConf> partitions{};
  for (const std::string& c : Split(s, '\n')) {
    std::optional<PartitionConf> cfg = PartitionConf::Parse(c);
    if (!cfg) {
      return std::nullopt;
    }
    partitions.push_back(*cfg);
  }
  return Conf(partitions);
}

PartitionConf::PartitionConf(Type type, uint32_t id,
                             const std::filesystem::path& path,
                             const std::string& addr, uint16_t port)
    : type_{type}, id_{id}, path_{path}, addr_{addr}, port_{port} {}

bool PartitionConf::operator==(const PartitionConf& cfg) const {
  return type_ == cfg.type_ && id_ == cfg.id_ && path_ == cfg.path_ &&
         addr_ == cfg.addr_ && port_ == cfg.port_;
}

bool PartitionConf::operator!=(const PartitionConf& cfg) const {
  return !(*this == cfg);
}

std::optional<PartitionConf> PartitionConf::Parse(const std::string& s) {
  const std::vector<std::string> fields = Split(s, ':');
  if (fields.size() != 5) {
    LOG(ERROR) << "partition config invalid number of fields";
    return std::nullopt;
  }

  PartitionConf cfg;
  if (!ParseType(fields[0])) return std::nullopt;
  cfg.type_ = *ParseType(fields[0]);

  if (!ParsePort(fields[1])) return std::nullopt;
  cfg.id_ = *ParseId(fields[1]);

  cfg.path_ = fields[2];

  cfg.addr_ = fields[3];

  if (!ParsePort(fields[4])) return std::nullopt;
  cfg.port_ = *ParsePort(fields[4]);

  return cfg;
}

std::optional<PartitionConf::Type> PartitionConf::ParseType(
    const std::string& s) {
  PartitionConf cfg;
  if (s == "leader") {
    return PartitionConf::Type::kLeader;
  } else if (s == "replica") {
    return PartitionConf::Type::kReplica;
  }
  LOG(ERROR) << "partition type not recognized: " << s;
  return std::nullopt;
}

std::optional<uint16_t> PartitionConf::ParseId(const std::string& s) {
  try {
    uint32_t id = std::stoi(s);
    if (std::to_string(id) == s) {
      return id;
    } else {
      LOG(ERROR) << "partition config id would over/under-flow: " << s;
      return std::nullopt;
    }
  } catch (const std::invalid_argument& e) {
    LOG(ERROR) << "partition config id not a number: " << s;
    return std::nullopt;
  }
}

std::optional<uint16_t> PartitionConf::ParsePort(const std::string& s) {
  try {
    uint16_t port = std::stoi(s);
    if (std::to_string(port) == s) {
      return port;
    } else {
      LOG(ERROR) << "partition config port would over/under-flow: " << s;
      return std::nullopt;
    }
  } catch (const std::invalid_argument& e) {
    LOG(ERROR) << "partition config port not a number: " << s;
    return std::nullopt;
  }
}

std::vector<std::string> Split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(s);
  while (std::getline(token_stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

}  // namespace wombat::broker
