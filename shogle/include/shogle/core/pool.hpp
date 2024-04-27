#pragma once

#include <shogle/core/loader.hpp>

#include <tuple>

namespace ntf {

template<typename T, typename... TRes>
concept same_as_defined = (... or std::same_as<T, TRes>);

template<typename... TRes>
class ResPool {
private:
  template<typename T>
  using rescontainer_t = std::unordered_map<fs::ResLoader::resid_t, T>;

  using pool_t = std::tuple<rescontainer_t<TRes>...>;

public:
  using resid_t = fs::ResLoader::resid_t;
  using pathinfo_t = fs::ResLoader::pathinfo_t;
  using reqcallback_t = fs::ResLoader::reqcallback_t;

public: // Resources can't be copied, so the pool can't be copied
  ResPool() = default;
  ~ResPool() = default;

  ResPool(ResPool&&) = default;
  ResPool(const ResPool&) = delete;
  ResPool& operator=(ResPool&&) = default;
  ResPool& operator=(const ResPool&) = delete;

public:
  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  inline const TReq* get(resid_t id) {
    return &std::get<rescontainer_t<TReq>>(_pool).at(id);
  }

  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  inline void emplace(resid_t id, TReq::data_t* data) {
    auto& container = std::get<rescontainer_t<TReq>>(_pool);
    container.emplace(std::make_pair(id, TReq{data}));
  }

public: // Resource requesters
  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void direct_request(pathinfo_t pathinfo) {
    direct_request<TReq>({pathinfo});
  }

  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void direct_request(std::initializer_list<pathinfo_t> pathinfo_list) {
    for (const auto& res_info : pathinfo_list) {
      auto& loader = fs::ResLoader::instance();
      auto data_ptr = loader.direct_load<TReq>(res_info);
      emplace<TReq>(res_info.id, data_ptr.get());
    }
  }

  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void async_request(pathinfo_t pathinfo, reqcallback_t on_load) {
    async_request<TReq>({pathinfo}, std::move(on_load));
  }

  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void async_request(std::initializer_list<pathinfo_t> pathinfo_list, reqcallback_t on_load) {
    _load_counters.push_back(std::make_pair(pathinfo_list.size(), 0));
    auto* counter = &_load_counters.back();

    for (const auto& res_info : pathinfo_list) {
      auto& loader = fs::ResLoader::instance();

      loader.async_load<TReq>(res_info, [this, counter, on_load](auto id, auto data_ptr) {
        size_t res_total = counter->first;
        size_t& res_c = counter->second;
        emplace<TReq>(id, data_ptr.get());
        if (++res_c == res_total) { on_load(); }
      });
    }
  }

private:
  pool_t _pool;
  std::vector<std::pair<size_t, size_t>> _load_counters;
};

} // namespace ntf
