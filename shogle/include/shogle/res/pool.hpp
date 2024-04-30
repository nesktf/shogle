#pragma once

#include <shogle/res/loader.hpp>
#include <shogle/core/log.hpp>

#include <tuple>

namespace ntf::res {

template<typename T, typename... TRes>
concept same_as_defined = (... or std::same_as<T, TRes>);

template<typename T>
concept uses_loader = requires { T::loader_t; };

template<typename... T>
// requires(uses_loader<T> && ...)
class pool {
private:
  template<typename _T>
  using rescontainer_t = std::unordered_map<loader::resid_t, _T>;

  using pool_t = std::tuple<rescontainer_t<T>...>;

public:
  using resid_t = loader::resid_t;
  using pathinfo_t = loader::pathinfo_t;
  using reqcallback_t = loader::reqcallback_t;

public: // Resources can't be copied, so the pool can't be copied
  pool() = default;
  ~pool() = default;

  pool(pool&&) = default;
  pool(const pool&) = delete;
  pool& operator=(pool&&) = default;
  pool& operator=(const pool&) = delete;

public:
  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  inline TReq* get(resid_t id) {
    auto& cont = std::get<rescontainer_t<TReq>>(_pool);
    return &cont.at(id);
  }

  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  inline void emplace(resid_t id, TReq::loader_t data) {
    auto& container = std::get<rescontainer_t<TReq>>(_pool);
    container.emplace(std::make_pair(id, TReq{std::move(data)}));
  }

public: // Resource requesters
  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  void direct_request(pathinfo_t pathinfo) {
    direct_request<TReq>({pathinfo});
  }

  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  void direct_request(std::initializer_list<pathinfo_t> pathinfo_list) {
    for (const auto& res_info : pathinfo_list) {
      auto& loader = loader::instance();
      emplace<TReq>(res_info.id, loader.direct_load<TReq>(res_info));
    }
  }

  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  void async_request(pathinfo_t pathinfo, reqcallback_t on_load) {
    async_request<TReq>({pathinfo}, std::move(on_load));
  }

  template<typename TReq>
  requires(same_as_defined<TReq, T...>)
  void async_request(std::initializer_list<pathinfo_t> pathinfo_list, reqcallback_t on_load) {
    _load_counters.push_back(std::make_pair(pathinfo_list.size(), 0));
    auto* counter = &_load_counters.back();

    for (const auto& res_info : pathinfo_list) {
      auto& loader = loader::instance();

      loader.async_load<TReq>(res_info, [this, counter, on_load](auto id, auto data_ptr) {
        size_t res_total = counter->first;
        size_t& res_c = counter->second;
        emplace<TReq>(id, *data_ptr.get());
        if (++res_c == res_total) { on_load(); }
      });
    }
  }

private:
  pool_t _pool;
  std::vector<std::pair<size_t, size_t>> _load_counters;
};

} // namespace ntf::res
