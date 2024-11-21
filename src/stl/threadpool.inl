#define SHOGLE_STL_THREADPOOL_INL
#include "./threadpool.hpp"
#undef SHOGLE_STL_THREADPOOL_INL

namespace ntf {

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
