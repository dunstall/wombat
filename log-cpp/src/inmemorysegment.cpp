#include "log/inmemorysegment.h"

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include "log/segment.h"

namespace wombat::log {

// Store in-memory segment state in a singeton to act like a filesystem.
class InMemorySegmentState {
 public:
  InMemorySegmentState() {}

  InMemorySegmentState(const InMemorySegmentState&) = delete;

  void operator=(const InMemorySegmentState&)  = delete;

  static InMemorySegmentState& GetInstance() {
    static InMemorySegmentState instance{};
    return instance;
  }

  std::unordered_map<std::string, std::vector<uint8_t>>& state() {
    return state_;
  }

 private:
  std::unordered_map<std::string, std::vector<uint8_t>> state_;
};

InMemorySegment::InMemorySegment(
    uint64_t id,
    const std::filesystem::path& path,
    size_t limit
) : Segment(limit), path_{path / IdToName(id)} {
  auto& state = InMemorySegmentState().GetInstance().state();
  if (state.find(path_.string()) == state.end()) {
    state.emplace(path_.string(), std::vector<uint8_t>{});
  } else {
    size_ = state.at(path_.string()).size();
  }
}

void InMemorySegment::Append(const std::vector<uint8_t>& data) {
  std::vector<uint8_t>& buf = InMemorySegmentState().GetInstance().state().at(path_.string());
  buf.insert(buf.end(), data.begin(), data.end());
  size_ += data.size();
}

std::vector<uint8_t> InMemorySegment::Lookup(uint64_t offset, uint64_t size) {
  std::vector<uint8_t>& buf = InMemorySegmentState().GetInstance().state().at(path_.string());
  if (buf.size() < offset + size) return {};
  return std::vector(buf.begin() + offset, buf.begin() + offset + size);
};

}  // namespace wombat::log
