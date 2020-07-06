// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <filesystem>
#include <list>
#include <optional>
#include <string>

namespace wombat::broker {

class PartitionConf;

class Conf {
 public:
  explicit Conf(std::list<PartitionConf> partitions);

  std::list<PartitionConf> partitions() const { return partitions_; }

  static std::optional<Conf> Parse(const std::string& s);

 private:
  std::list<PartitionConf> partitions_;
};

class PartitionConf {
 public:
  enum class Type {
    kLeader,
    kReplica
  };

  PartitionConf() = default;
  PartitionConf(Type type,
                const std::filesystem::path& path,
                const std::string& addr,
                uint16_t port);

  Type type() const { return type_; }

  std::filesystem::path path() const { return path_; }

  std::string addr() const { return addr_; }

  uint16_t port() const { return port_; }

  bool operator==(const PartitionConf& cfg) const;
  bool operator!=(const PartitionConf& cfg) const;

  static std::optional<PartitionConf> Parse(const std::string& s);

 private:
  static std::optional<Type> ParseType(const std::string& s);

  static std::optional<uint16_t> ParsePort(const std::string& s);

  Type type_;
  std::filesystem::path path_;
  std::string addr_;
  uint16_t port_;
};

}  // namespace wombat::broker
