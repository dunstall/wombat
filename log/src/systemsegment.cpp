#include "log/systemsegment.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <filesystem>

#include "log/logexception.h"

namespace wombat::log {

SystemSegment::SystemSegment(
    uint32_t id,
    const std::filesystem::path& dir,
    uint32_t limit)
  : Segment{dir / IdToName(id), limit}
{
  std::filesystem::create_directories(dir);

  // Note cannot use O_APPEND as this does not work with sendfile.
  // TODO(AD) O_ASYNC and O_NONBLOCK
  fd_ = open(path_.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd_ == -1) {
    throw LogException{"failed to open segment", errno};
  }

  size_ = Size();
}

SystemSegment::~SystemSegment() {
  close(fd_);
}

}  // namespace wombat::log
