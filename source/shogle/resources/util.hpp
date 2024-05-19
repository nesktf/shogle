#pragma once

#include <shogle/core/threadpool.hpp>
#include <shogle/core/types.hpp>

#include <tuple>

namespace ntf::shogle::resources {

std::string file_contents(std::string path);
std::string file_dir(std::string path);

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

template<typename T, typename... TRes>
concept same_as_defined = (... or std::same_as<T, TRes>);

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
  requires(same_as_defined<T, pool_types...>)
  inline wptr<T> get(id_t id) {
    return &std::get<map_t<T>>(_pool).at(id);
  }

  template<typename T, typename... Args>
  requires(same_as_defined<T, pool_types...>)
  inline void emplace(id_t id, Args&&... args) {
    std::get<map_t<T>>(_pool).emplace(
      std::make_pair(id, T{std::forward<Args>(args)...})
    );
  }

  template<typename T>
  requires(same_as_defined<T, pool_types...>)
  inline void clear_pool(void) {
    std::get<map_t<T>>(_pool).clear();
  }

public:
  template<typename T>
  requires(same_as_defined<T, pool_types...>)
  void direct_request(std::initializer_list<path_info> list) {
    using data_t = T::data_t;
    for (const auto& info : list) {
      std::get<map_t<T>>(_pool).emplace(
        std::make_pair(info.id, data_t{info.path})
      );
    }
  }

  template<typename T>
  requires(same_as_defined<T, pool_types...>)
  void async_request(async_loader& loader, load_callback on_load, std::initializer_list<path_info> list) {
    using data_t = T::data_t;

    _load_counters.push_back(std::make_pair(list.size(), 0));
    auto* c_it = &_load_counters.back();

    for (const auto& info : list) {
      auto id = info.id;
      loader.add_request<data_t>([this, c_it, id=std::move(id), on_load](data_t data) {
        size_t res_total = c_it->first;
        size_t& res_c = c_it->second;

        std::get<map_t<T>>(_pool).emplace(
          std::make_pair(std::move(id), std::move(data))
        );
        if (++res_c == res_total) { on_load(); }
      }, info.path);
    }
  }

private:
  pool_t _pool;
  std::vector<std::pair<size_t, size_t>> _load_counters;
};

} // namespace ntf::shogle::resources
