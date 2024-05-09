#pragma once

#include <shogle/core/threadpool.hpp>
#include <shogle/core/types.hpp>

#include <memory>
#include <functional>

namespace ntf::res {

class async_loader {
public:
  using reqfun_t = std::function<void()>;

private:
  template<typename T>
  using loadfun_t = std::function<void(T)>;

  template<typename T>
  struct load_wrapper { // to be used as a reqfun_t
    loadfun_t<T> _on_load;
    T* _data; // has to be in the heap because of std::function

    template<typename... Args>
    load_wrapper(loadfun_t<T> fun, Args&&... args) :
      _on_load(std::move(fun)),
      _data(new T{std::forward<Args>(args)...}) {}

    void operator()(void) {
      _on_load(std::move(*_data));

      // when the callback fires we no longer need the memory
      delete _data;
    }
  };

public:
  async_loader() = default;
  async_loader(size_t n_threads) :
    _thpool(n_threads) {}

public:
  inline void do_requests(void) {
    while (!_req.empty()) {
      auto req_callback = std::move(_req.front());
      _req.pop();
      req_callback();
    }
  }

public:
  template<typename T, typename... Args>
  void add_request(loadfun_t<T> on_load, Args&&... load_args);

private:
  std::mutex _req_mtx;
  std::queue<reqfun_t> _req;
  thread_pool _thpool;
};


template<typename T, typename... Args>
void async_loader::add_request(loadfun_t<T> on_load, Args&&... load_args) {
  // thank you based c++20
  _thpool.enqueue([this, on_load=std::move(on_load), ...args=std::forward<Args>(load_args)]() {
    load_wrapper<T> loader { std::move(on_load), std::forward<Args>(args)...};
    std::unique_lock lock{_req_mtx};
    _req.emplace(std::move(loader));
  });
}

} // namespace ntf::res
