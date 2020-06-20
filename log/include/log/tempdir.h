#include <iostream>
#include <random>
#include <string>

namespace wombat::log {

const size_t PATH_LEN = 6;
const std::string CHARS = "0123456789abcdefghijklmnopqrstuvwxyz";

static std::filesystem::path GeneratePath() {
  thread_local static std::mt19937 rg{std::random_device{}()};
  thread_local static std::uniform_int_distribution<std::string::size_type> pick(
    0, CHARS.size() - 2
  );
  std::string dir;
  dir.reserve(PATH_LEN);
  for (size_t i = 0; i != PATH_LEN; ++i) {
    dir += CHARS[pick(rg)];
  }
  return "/tmp/wombatlog" + dir;
}

class TempDir {
 public:
  TempDir() : path_(GeneratePath()) {
    std::filesystem::create_directories(path_);
  }

  ~TempDir() {
    std::filesystem::remove_all(path_);
  }

  std::filesystem::path path() const { return path_; }

 private:
  std::filesystem::path path_;
};

}  // namespace wombat::log
