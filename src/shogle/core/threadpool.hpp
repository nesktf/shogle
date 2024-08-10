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
  inline thread_pool(size_t n_threads = std::thread::hardware_concurrency());

  inline ~thread_pool();
  thread_pool(thread_pool&&) = delete;
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(thread_pool&&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

public:
  inline void enqueue(task_t task);

private:
  bool _stop {false};

  std::vector<std::thread> _threads;

  std::queue<task_t> _tasks;
  std::mutex _task_mtx;
  std::condition_variable _cv;
};

thread_pool::thread_pool(size_t n_threads) {
  for (size_t i = 0; i < n_threads; ++i) {
    _threads.emplace_back([this]() { while(true) {
      task_t task;

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

thread_pool::~thread_pool() {
  {
    std::unique_lock<std::mutex> lock(_task_mtx);
    _stop = true;
  }

  _cv.notify_all();

  for (auto& thread : _threads) {
    thread.join();
  }
}

void thread_pool::enqueue(task_t task) {
  {
    std::unique_lock<std::mutex> lock(_task_mtx);
    _tasks.emplace(std::move(task));
  }
  _cv.notify_one();
}

} // namespace ntf
