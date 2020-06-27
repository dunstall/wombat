#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace wombat::broker::util {

template<class T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() : data_queue_{} {}

  void Push(T val) {
    std::lock_guard<std::mutex> lk(mut_);
    data_queue_.push(val);
    data_cond_.notify_one();
  }

  T WaitAndPop() {
    std::unique_lock<std::mutex> lk(mut_);
    data_cond_.wait(lk, [this]{ return !data_queue_.empty(); });
    T val = std::move(data_queue_.front());
    data_queue_.pop();
    return val;
  }

  bool TryPop(T* val) {
    std::lock_guard<std::mutex> lk(mut_);
    if (data_queue_.empty()) {
      return false;
    }
    *val = std::move(data_queue_.front());
    data_queue_.pop();
    return true;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(mut_);
    return data_queue_.empty();
  }

 private:
  std::queue<T> data_queue_;

  std::condition_variable data_cond_;

  std::mutex mut_;
};

}  // namespace wombat::broker::util
