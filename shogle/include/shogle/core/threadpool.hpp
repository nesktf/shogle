#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <vector>

namespace ntf {

class thread_pool {
public:
  using task_t = std::function<void()>;

public:
  thread_pool(size_t n_threads = std::thread::hardware_concurrency());
  ~thread_pool();

  thread_pool(thread_pool&&) = delete;
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(thread_pool&&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

public:
  void enqueue(task_t task);

private:
  bool _stop {false};

  std::vector<std::thread> _threads;

  std::queue<task_t> _tasks;
  std::mutex _task_mtx;
  std::condition_variable _cv;
};

} // namespace ntf
