#include <shogle/core/threadpool.hpp>

namespace ntf {

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
