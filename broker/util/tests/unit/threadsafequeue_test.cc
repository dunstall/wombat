// Copyright 2020 Andrew Dunstall

#include <chrono>
#include <future>
#include <thread>

#include "gtest/gtest.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::util {

using namespace std::chrono_literals;  // NOLINT

class ThreadSafeQueueTest : public ::testing::Test {};

TEST_F(ThreadSafeQueueTest, Empty) {
  ThreadSafeQueue<int> queue{};
  EXPECT_TRUE(queue.empty());
}

TEST_F(ThreadSafeQueueTest, WaitAndPopOk) {
  int val;
  ThreadSafeQueue<int> queue{};
  queue.Push(val);
  EXPECT_EQ(val, queue.WaitAndPop());
}

TEST_F(ThreadSafeQueueTest, WaitAndPopFillsWhileWaiting) {
  int val;
  ThreadSafeQueue<int> queue{};
  std::async(std::launch::async, [&] {
    std::this_thread::sleep_for(10ms);
    queue.Push(val);
  });
  EXPECT_EQ(val, queue.WaitAndPop());
}

TEST_F(ThreadSafeQueueTest, WaitForAndPopOk) {
  int val;
  ThreadSafeQueue<int> queue{};
  queue.Push(val);
  EXPECT_EQ(val, queue.WaitForAndPop(100ms));
}

TEST_F(ThreadSafeQueueTest, WaitForAndPopEmpty) {
  ThreadSafeQueue<int> queue{};
  EXPECT_FALSE(queue.WaitForAndPop(10ms));
}

TEST_F(ThreadSafeQueueTest, WaitForAndPopFillsWhileWaiting) {
  int val;
  ThreadSafeQueue<int> queue{};
  std::async(std::launch::async, [&] {
    std::this_thread::sleep_for(10ms);
    queue.Push(val);
  });
  EXPECT_EQ(val, queue.WaitForAndPop(50ms));
}

TEST_F(ThreadSafeQueueTest, TryPopOk) {
  int val;
  ThreadSafeQueue<int> queue{};
  queue.Push(val);

  std::optional<int> res = queue.TryPop();
  EXPECT_TRUE(res);
  EXPECT_EQ(*res, val);
}

TEST_F(ThreadSafeQueueTest, TryPopEmpty) {
  ThreadSafeQueue<int> queue{};
  EXPECT_FALSE(queue.TryPop());
}

}  // namespace wombat::broker::util
