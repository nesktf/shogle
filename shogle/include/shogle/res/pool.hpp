#pragma once

#include <shogle/res/loader.hpp>
#include <shogle/core/log.hpp>

#include <tuple>

namespace ntf::res {

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
  inline T* get(id_t id) {
    return &std::get<map_t<T>>(_pool).at(id);
  }

public:
  template<typename T>
  requires(same_as_defined<T, pool_types...>)
  void direct_request(std::initializer_list<path_info> list) {
    using loader_t = T::loader_t;
    for (const auto& info : list) {
      std::get<map_t<T>>(_pool).emplace(
        std::make_pair(info.id, loader_t{info.path})
      );
    }
  }

  template<typename T>
  requires(same_as_defined<T, pool_types...>)
  void async_request(async_loader& loader, load_callback on_load, std::initializer_list<path_info> list) {
    using loader_t = T::loader_t;

    _load_counters.push_back(std::make_pair(list.size(), 0));
    auto* c_it = &_load_counters.back();

    for (const auto& info : list) {
      auto id = info.id;
      loader.add_request<loader_t>([this, c_it, id=std::move(id), on_load](loader_t data) {
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

} // namespace ntf::res
