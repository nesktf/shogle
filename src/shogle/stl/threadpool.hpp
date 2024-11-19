#pragma once

#include "./common.hpp"

#include <functional>
#include <queue>
#include <condition_variable>

namespace ntf {

class thread_pool {
public:
  using task_type = std::function<void()>;

public:
  thread_pool(std::size_t n_threads = std::thread::hardware_concurrency());

public:
  void enqueue(task_type task);

private:
  bool _stop {false};

  std::vector<std::thread> _threads;

  std::queue<task_type> _tasks;
  std::mutex _task_mtx;
  std::condition_variable _cv;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(thread_pool);
};

} // namespace ntf

#ifndef SHOGLE_STL_THREADPOOL_INL
#include "./threadpool.inl"
#endif
