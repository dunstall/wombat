// Copyright 2020 Andrew Dunstall

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <streambuf>

#include "broker/conf.h"
#include "broker/router.h"
#include "event/event.h"
#include "event/responder.h"
#include "glog/logging.h"
#include "log/log.h"
#include "log/systemlog.h"
#include "partition/partition.h"
#include "partition/leader.h"
#include "server/server.h"
#include "util/threadable.h"

namespace wombat::broker {

const std::filesystem::path kDefaultPath = "/usr/local/wombat/WOMBAT.conf";

constexpr uint16_t kPort = 3110;

std::optional<Conf> ParseConf(const std::filesystem::path& path) {
  std::ifstream f(path);
  if (!f.is_open()) return std::nullopt;
  const std::string s(
      (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()
  );
  return Conf::Parse(s);
}

void Run(const std::filesystem::path& path) {
  LOG(INFO) << "running wombat broker";

  std::optional<Conf> cfg = ParseConf(path);
  if (!cfg) {
    LOG(ERROR) << "failed to parse broker config at " << path;
    std::exit(EXIT_FAILURE);
  }

  Router router{};
  for (const PartitionConf& p : cfg->partitions()) {
    LOG(INFO) << "adding partition " << p.id();

    std::shared_ptr<log::Log> log = std::make_shared<log::SystemLog>(p.path());

    switch (p.type()) {
      case PartitionConf::Type::kLeader:
        router.AddPartition(
            std::make_unique<partition::Leader>(
                p.id(), std::make_shared<Responder>(), log
            )
        );
        break;
      case PartitionConf::Type::kReplica:
        // TODO(AD) Replica not yet supported.
        // router.AddPartition(
        //    std::make_unique<Replica>(...)
        //);
        break;
    }
  }
  std::shared_ptr<server::Server> server
      = std::make_shared<server::Server>(kPort);
  util::Threadable threadable_server(server);

  while (true) {
    router.Route(server->events()->WaitAndPop());
  }
}

}  // namespace wombat::broker

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  wombat::broker::Run((argc < 2) ? wombat::broker::kDefaultPath : argv[1]);
}
