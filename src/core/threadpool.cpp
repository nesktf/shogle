#include "core/threadpool.hpp"

namespace ntf {

ThreadPool::ThreadPool(size_t n_threads) {
  this->stop = false;

  for (size_t i = 0; i < n_threads; ++i) {
    threads.emplace_back([this] {
      while(true) {
        std::function<void()> task;

        {
          std::unique_lock<std::mutex> lock(queue_mutex);
          cv.wait(lock, [this] {
            return !tasks.empty() || this->stop;
          });

          if (this->stop && tasks.empty()) {
            return;
          }

          task = std::move(tasks.front());
          tasks.pop();
        }

        task();
      }
    });
  }

}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }

  cv.notify_all();

  for (auto& thread : threads) {
    thread.join();
  }
}

void ThreadPool::enqueue(std::function<void()> task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.emplace(std::move(task));
  }
  cv.notify_one();
}

} // namespace ntf
