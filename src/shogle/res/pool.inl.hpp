#define POOL_INL_HPP
#include <shogle/res/pool.hpp>
#undef POOL_INL_HPP

namespace ntf::shogle {

inline void async_loader::do_requests() {
  while (!_req.empty()) {
    auto req_callback = std::move(_req.front());
    _req.pop();
    req_callback();
  }
}

template<typename T, typename... Args>
void async_loader::add_request(loadfun_t<T> on_load, Args&&... load_args) {
  // thank you based c++20
  _thpool.enqueue([this, on_load=std::move(on_load), ...args=std::forward<Args>(load_args)]() {
    load_wrapper<T> loader { std::move(on_load), std::forward<Args>(args)...};
    std::unique_lock lock{_req_mtx};
    _req.emplace(std::move(loader));
  });
}

template<typename... pool_types>
template<typename T>
T* pool<pool_types...>::get(id_t id) {
  return &std::get<map_t<T>>(_pool).at(id);
}

template<typename... pool_types>
template<typename T>
void pool<pool_types...>::clear() {
  std::get<map_t<T>>(_pool).clear();
}

template<typename... pool_types>
template<typename T>
void pool<pool_types...>::direct_request(std::initializer_list<path_info> list) {
  using data_t = T::data_t;
  for (const auto& info : list) {
    std::get<map_t<T>>(_pool).emplace(
      std::make_pair(info.id, data_t{info.path})
    );
  }
}

template<typename... pool_types>
template<typename T>
void pool<pool_types...>::async_request(async_loader& loader, load_callback on_load, std::initializer_list<path_info> list) {
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

} // namespace ntf::shogle
