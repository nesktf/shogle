#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <vector>

namespace ntf {

class ThreadPool {
public:
  ThreadPool(size_t n_threads = std::thread::hardware_concurrency());
  ~ThreadPool();

  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

public:
  void enqueue(std::function<void()> task);

private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop;
};

} // namespace ntf
