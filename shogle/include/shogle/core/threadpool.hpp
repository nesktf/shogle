#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <vector>

namespace ntf {

class ThreadPool {
public:
  using task_t = std::function<void()>;

public:
  ThreadPool(size_t n_threads = std::thread::hardware_concurrency());
  ~ThreadPool();

  ThreadPool(ThreadPool&&) = delete;
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

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
