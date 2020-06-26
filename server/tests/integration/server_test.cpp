#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "server/server.h"

namespace wombat::log {

class BackgroundServer {
 public:
  BackgroundServer(Server server) : server_{std::move(server)} {}

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

 private:
  void Poll() {
    while (running_) {
      server_.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  Server server_;

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
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout));

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
  setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock1, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock3, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
  setsockopt(sock3, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout));

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

}  // namespace wombat::log
