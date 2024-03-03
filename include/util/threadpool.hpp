#pragma once

#include <cstddef>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <vector>

namespace ntf::shogle {

class ThreadPool {
public:
  ThreadPool(size_t n_threads = std::thread::hardware_concurrency());
  ~ThreadPool();

  void enqueue(std::function<void()> task);

private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop;

};

}
