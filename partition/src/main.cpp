#include "partition/leader.h"

#include <chrono>
#include <thread>

#include <glog/logging.h>
#include "partition/leader.h"
#include "log/log.h"
#include "log/logexception.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

void Run() {
  TempDir dir{};
  std::shared_ptr<Log<InMemorySegment>> log_leader = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);
  
  Leader<InMemorySegment> leader{log_leader, 3110};

  for (uint8_t b = 0; b != 0xff; ++b) {
    log_leader->Append({b, b, b, b, b});
  }

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    leader.Poll();
  }
}

}  // namespace wombat::log::testing

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "running leader";
  wombat::log::testing::Run();
}
