#include "log/inmemorysegment.h"

#include <fcntl.h>
#include <sys/mman.h>

#include <string>
#include <unordered_map>

#include "log/logexception.h"

namespace wombat::broker {

// Store in-memory segment state (file descripor) in a singeton to act like a
// persistent filesystem.
class InMemorySegmentState {
 public:
  InMemorySegmentState() {}

  InMemorySegmentState(const InMemorySegmentState&) = delete;

  static InMemorySegmentState& GetInstance() {
    static InMemorySegmentState instance{};
    return instance;
  }

  std::unordered_map<std::string, int>& state() {
    return state_;
  }

 private:
  std::unordered_map<std::string, int> state_;
};

InMemorySegment::InMemorySegment(
    uint32_t id,
    const std::filesystem::path& dir,
    uint32_t limit
) : Segment{dir / IdToName(id), limit} {
  auto& state = InMemorySegmentState().GetInstance().state();
  if (state.find(path_.string()) == state.end()) {
    fd_ = memfd_create(path_.c_str(), O_RDWR);
    if (fd_ == -1) {
      throw LogException{"memfd_create failed", errno};
    }
    state.emplace(path_.string(), fd_);
  } else {
    fd_ = state.at(path_.string());
  }

  size_ = Size();
}

InMemorySegment::~InMemorySegment() {
  // Do not close fd_ as this would destroy the in-memory state. This is
  // ok as only for testing.
}

}  // namespace wombat::broker
