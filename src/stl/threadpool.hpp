#pragma once

#include "./types.hpp"

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

inline thread_pool::thread_pool(std::size_t n_threads) {
  for (size_t i = 0; i < n_threads; ++i) {
    _threads.emplace_back([this]() { while(true) {
      task_type task;

      {
        std::unique_lock<std::mutex> lock(_task_mtx);
        _cv.wait(lock, [this]() {
          return !_tasks.empty() || _stop;
        });

        if (_stop && _tasks.empty()) {
          return;
        }

        task = std::move(_tasks.front());
        _tasks.pop();
      }

      task();
    }});
  }
}

inline thread_pool::~thread_pool() noexcept {
  {
    std::unique_lock<std::mutex> lock(_task_mtx);
    _stop = true;
  }

  _cv.notify_all();

  for (auto& thread : _threads) {
    thread.join();
  }
}

inline void thread_pool::enqueue(task_type task) {
  {
    std::unique_lock<std::mutex> lock(_task_mtx);
    _tasks.emplace(std::move(task));
  }
  _cv.notify_one();
}

} // namespace ntf
