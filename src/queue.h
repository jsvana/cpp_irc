#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class queue {
 private:
  std::mutex q_lock_;
  std::condition_variable cv_;
  std::queue<T> q_;

 public:
  void push(T value) {
    std::lock_guard<std::mutex> guard(q_lock_);
    cv_.notify_all();
    q_.push(value);
  }

  T pop() {
    std::unique_lock<std::mutex> lock(q_lock_);
    cv_.wait(lock, [this]{ return !q_.empty(); });
    auto ret = q_.front();
    q_.pop();
    return ret;
  }

  bool empty() {
    std::lock_guard<std::mutex> guard(q_lock_);
    return q_.empty();
  }
};
