// Copyright 2020 Andrew Dunstall

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "gtest/gtest.h"
#include "record/producerecord.h"
#include "server/server.h"

namespace wombat::broker::server {

class BackgroundServer {
 public:
  explicit BackgroundServer(Server server) : server_{std::move(server)} {}

  ~BackgroundServer() {
    Stop();
  }

  void Start() {
    running_ = true;
    thread_ = std::thread{&BackgroundServer::Poll, this};
  }

  void Stop() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  std::vector<record::ProduceRecord> Received() {
    std::lock_guard<std::mutex> lk(mtx_);
    return received_;
  }

 private:
  void Poll() {
    while (running_) {
      const std::vector<record::ProduceRecord> recv = server_.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      std::lock_guard<std::mutex> lk(mtx_);
      received_.insert(received_.end(), recv.begin(), recv.end());
    }
  }

  Server server_;

  std::vector<record::ProduceRecord> received_;

  std::mutex mtx_;
  std::thread thread_;
  std::atomic_bool running_;
};

class ServerTest : public ::testing::Test {
 protected:
  const std::string kLocalhost = "127.0.0.1";
};

TEST_F(ServerTest, TestConnectOk) {
  const uint16_t port = 4100;

  BackgroundServer server{Server{port}};
  server.Start();

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, kLocalhost.c_str(), &servaddr.sin_addr.s_addr);

  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT

  EXPECT_NE(connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)), -1);

  std::vector<uint8_t> buf(5);
  EXPECT_EQ(read(sock, buf.data(), 5), -1);
  EXPECT_EQ(EWOULDBLOCK, errno);

  close(sock);

  server.Stop();
}

TEST_F(ServerTest, TestConnectExceedClientLimit) {
  const uint16_t port = 4101;

  BackgroundServer server{Server{port, 1}};
  server.Start();

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, kLocalhost.c_str(), &servaddr.sin_addr.s_addr);

  int sock1 = socket(AF_INET, SOCK_STREAM, 0);
  int sock2 = socket(AF_INET, SOCK_STREAM, 0);
  int sock3 = socket(AF_INET, SOCK_STREAM, 0);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock1, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock3, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock3, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT

  EXPECT_NE(connect(sock1, (struct sockaddr*) &servaddr, sizeof(servaddr)), -1);

  std::vector<uint8_t> buf(5);
  EXPECT_EQ(read(sock1, buf.data(), 5), -1);
  EXPECT_EQ(EWOULDBLOCK, errno);

  EXPECT_NE(connect(sock2, (struct sockaddr*) &servaddr, sizeof(servaddr)), -1);

  // Connection should be immediately closed by the server.
  EXPECT_EQ(0, read(sock2, buf.data(), 5));

  close(sock1);
  close(sock2);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  // After closing sock1 should connect successfully.
  EXPECT_NE(connect(sock3, (struct sockaddr*) &servaddr, sizeof(servaddr)), -1);
  EXPECT_EQ(read(sock3, buf.data(), 5), -1);
  EXPECT_EQ(EWOULDBLOCK, errno);

  close(sock3);

  server.Stop();
}

TEST_F(ServerTest, TestSendProduceRecord) {
  const uint16_t port = 4105;

  BackgroundServer server{Server{port}};
  server.Start();

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, kLocalhost.c_str(), &servaddr.sin_addr.s_addr);

  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));  // NOLINT

  connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr));

  // Write record at once.
  // TODO(AD) Need test for writing one byte at a time (see test harness)
  const std::vector<uint8_t> data{1, 2, 3};
  const record::ProduceRecord record{data};
  const std::vector<uint8_t> encoded = record.Encode();
  EXPECT_EQ((int) encoded.size(), write(sock, encoded.data(), encoded.size()));

  // TODO(AD) just have the poll in the test rather than background?
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(1U, server.Received().size());
  EXPECT_EQ(record, server.Received()[0]);

  close(sock);

  server.Stop();
}

}  // namespace wombat::broker::server
