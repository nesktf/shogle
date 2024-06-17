#pragma once
#include <shogle/core/threadpool.hpp>
#include <shogle/core/types.hpp>

#include <tuple>

namespace ntf::shogle {

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
  inline void do_requests();

public:
  template<typename T, typename... Args>
  void add_request(loadfun_t<T> on_load, Args&&... load_args);

private:
  std::mutex _req_mtx;
  std::queue<reqfun_t> _req;
  thread_pool _thpool;
};


template<typename... pool_types>
class pool {
public:
  using id_t = std::string;
  using load_callback = async_loader::reqfun_t;

  struct path_info {
    id_t id;
    path_t path;
  };

private:
  template<typename T>
  using map_t = std::unordered_map<id_t, T>;

  using pool_t = std::tuple<map_t<pool_types>...>;

public:
  pool() = default;

public:
  template<typename T>
  T* get(id_t id);

  template<typename T>
  void clear();

  template<typename T>
  void direct_request(std::initializer_list<path_info> list);

  template<typename T>
  void async_request(async_loader& loader, load_callback on_load, std::initializer_list<path_info> list);

private:
  pool_t _pool;
  std::vector<std::pair<size_t, size_t>> _load_counters;
};

} // namespace ntf::shogle

#ifndef POOL_INL_HPP
#include <shogle/res/pool.inl.hpp>
#endif

