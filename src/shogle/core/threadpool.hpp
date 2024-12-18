#pragma once

#include <shogle/core/common.hpp>

namespace ntf {

class thread_pool {
public:
  using task_t = std::function<void()>;

public:
  thread_pool(size_t n_threads = std::thread::hardware_concurrency());

public:
  void enqueue(task_t task);

private:
  bool _stop {false};

  std::vector<std::thread> _threads;

  std::queue<task_t> _tasks;
  std::mutex _task_mtx;
  std::condition_variable _cv;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(thread_pool);
};

} // namespace ntf

#ifndef NTF_THREADPOOL_INL
#include <shogle/core/threadpool.inl>
#endif
