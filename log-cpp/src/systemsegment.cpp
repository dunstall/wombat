#include "log/systemsegment.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <filesystem>

#include "log/logexception.h"

namespace wombat::log {

SystemSegment::SystemSegment(
    uint64_t id,
    const std::filesystem::path& path,
    size_t limit)
  : Segment(limit)
{
  std::filesystem::create_directories(path);

  const std::string name = IdToName(id);

  fs_.open(
      path / name,
      std::ios::in | std::ios::out | std::ios::binary | std::ios::app
  );
  size_ = std::filesystem::file_size(path / name);

  if (fs_.fail()) {
    throw LogException{"failed to open file"};
  }
}

void SystemSegment::Append(const std::vector<uint8_t>& data)
{
  size_ += data.size();
  fs_.write((char*) data.data(), data.size());
  if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to write to file"};
  }
  if (fs_.sync() == -1) {  // TODO(AD) only on timeout or size reached?
    throw LogException{"failed to sync file"};
  }
}

std::vector<uint8_t> SystemSegment::Lookup(uint64_t offset, uint64_t size)
{
  fs_.seekp(offset);
  if (fs_.eof()) {
    return {};
  } else if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to seek file"};
  }

  std::vector<uint8_t> data(size);
  fs_.read((char*) data.data(), size);
  if (fs_.eof()) {
    return {};
  } else if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to read file"};
  }

  return data;
}

}  // namespace wombat::log
