// Copyright 2020 Andrew Dunstall

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "frame/record.h"
#include "gtest/gtest.h"
#include "server/server.h"
#include "util/threadable.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

using namespace std::chrono_literals;  // NOLINT

class ServerTest : public ::testing::Test {
 protected:
  const std::string kLocalhost = "127.0.0.1";

  int CreateSocket() const;

  struct sockaddr_in ServerAddr(uint16_t port) const;

  uint16_t RandomPort() const;
};

TEST_F(ServerTest, TestConnectOk) {
  const uint16_t port = RandomPort();
  std::shared_ptr<Server> server = std::make_shared<Server>(port);
  util::Threadable threadable_server(server);

  struct sockaddr_in servaddr = ServerAddr(port);

  int sock = CreateSocket();
  EXPECT_NE(connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)), -1);

  std::vector<uint8_t> buf(5);
  EXPECT_EQ(read(sock, buf.data(), 5), -1);
  EXPECT_EQ(EWOULDBLOCK, errno);

  close(sock);
}

TEST_F(ServerTest, TestConnectExceedClientLimit) {
  const uint16_t port = RandomPort();
  std::shared_ptr<Server> server = std::make_shared<Server>(port, 1);
  util::Threadable threadable_server(server);
  struct sockaddr_in servaddr = ServerAddr(port);

  int sock1 = CreateSocket();
  int sock2 = CreateSocket();
  int sock3 = CreateSocket();

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
}

TEST_F(ServerTest, TestSendMessages) {
  const uint16_t port = RandomPort();
  std::shared_ptr<Server> server = std::make_shared<Server>(port);
  util::Threadable threadable_server(server);
  struct sockaddr_in servaddr = ServerAddr(port);

  int sock = CreateSocket();
  connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr));

  const std::vector<uint8_t> payload{1, 2, 3};
  const Message request{
    MessageType::kProduceRequest, 0, payload
  };
  const std::vector<uint8_t> encoded = request.Encode();

  const int n_requests = 3;
  for (int i = 0; i != n_requests; ++i) {
    EXPECT_EQ(
        (int) encoded.size(),
        write(sock, encoded.data(), encoded.size())
    );
  }

  for (int i = 0; i != n_requests; ++i) {
    std::optional<Event> e = server->events()->WaitForAndPop(100ms);
    ASSERT_TRUE(e);
    EXPECT_EQ(request, e->message);
  }

  close(sock);
}

TEST_F(ServerTest, TestSendMessagesOneByteAtATime) {
  const uint16_t port = RandomPort();
  std::shared_ptr<Server> server = std::make_shared<Server>(port);
  util::Threadable threadable_server(server);
  struct sockaddr_in servaddr = ServerAddr(port);

  int sock = CreateSocket();
  connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr));

  const std::vector<uint8_t> payload{1, 2, 3};
  const Message request{
      MessageType::kProduceRequest, 0, payload
  };
  const std::vector<uint8_t> encoded = request.Encode();

  const int n_requests = 3;
  for (int i = 0; i != n_requests; ++i) {
    for (size_t i = 0; i != encoded.size(); ++i) {
      // Just one byte - small sleep so server reads return less than the full
      // request.
      EXPECT_EQ(1, write(sock, encoded.data() + i, 1));
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
  }

  for (int i = 0; i != n_requests; ++i) {
    std::optional<Event> e = server->events()->WaitForAndPop(100ms);
    ASSERT_TRUE(e);
    EXPECT_EQ(request, e->message);
  }

  close(sock);
}

struct sockaddr_in ServerTest::ServerAddr(uint16_t port) const {
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, kLocalhost.c_str(), &servaddr.sin_addr.s_addr);
  return servaddr;
}

int ServerTest::CreateSocket() const {
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(
      sock, SOL_SOCKET, SO_RCVTIMEO,
      reinterpret_cast<char*>(&timeout), sizeof(timeout)
  );
  setsockopt(
      sock, SOL_SOCKET, SO_SNDTIMEO,
      reinterpret_cast<char*>(&timeout), sizeof(timeout)
  );

  return sock;
}

uint16_t ServerTest::RandomPort() const {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1024, 0xffff);
  uint16_t a =  distrib(gen);
  return a;
}

}  // namespace wombat::broker::server
