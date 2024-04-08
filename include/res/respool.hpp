#pragma once

#include "res/resloader.hpp"

#include <tuple>

namespace ntf {

template<typename T, typename... TRes>
concept same_as_defined = (... or std::same_as<T, TRes>);

template<typename... TRes>
class ResPool {
private:
  template<typename T> // Subject to change
  using ResContainer = std::unordered_map<id_t, T>;

  using LoadCallback = std::function<void()>;

public: // Resources can't be copied, so the pool can't be copied
  ResPool() = default;
  ~ResPool() = default;

  ResPool(ResPool&&) = default;
  ResPool& operator=(ResPool&&) = default;

  ResPool(const ResPool&) = delete;
  ResPool& operator=(const ResPool&) = delete;

public: // Resource getters
  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  TReq* get(id_t id) {
    return &std::get<ResContainer<TReq>>(pool).at(id);
  }

public: // Resource requesters
  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void direct_load(std::initializer_list<pathinfo_t> input_list) {
    for (const auto& res_info : input_list) {
      auto& loader = ResLoader::instance();
      auto data_ptr = loader.direct_load<TReq>(res_info);
      this->emplace<TReq>(res_info.id, data_ptr.get());
    }
  }

  template<typename TReq>
  requires(same_as_defined<TReq, TRes...>)
  void async_load(std::initializer_list<pathinfo_t> input_list, LoadCallback on_load) {
    load_counters.push_back(std::make_pair(input_list.size(), 0));
    auto* counter = &load_counters.back();

    for (const auto& res_info : input_list) {
      auto& loader = ResLoader::instance();

      loader.async_load<TReq>(res_info, [this, counter, on_load](auto id, auto data_ptr) {
        size_t res_total = counter->first;
        size_t& res_c = counter->second;
        this->emplace<TReq>(id, data_ptr.get());
        if (++res_c == res_total) {
          on_load();
        }
      });
    }
  }

public: // Place resource manually
  template<typename TReq>
  void emplace(id_t id, TReq::data_t* data) {
    auto& container = std::get<ResContainer<TReq>>(pool);
    container.emplace(std::make_pair(id, TReq{data}));
  }

private:
  std::tuple<ResContainer<TRes>...> pool;
  std::vector<std::pair<size_t, size_t>> load_counters;
};

} // namespace ntf
