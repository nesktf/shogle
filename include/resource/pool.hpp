#pragma once

#include "resource/loader.hpp"

#include "types.hpp"

#include <tuple>
#include <concepts>

namespace ntf::shogle::res {

template<typename T, typename... ResT>
concept same_as_defined = (... or std::same_as<T, ResT>);

template<typename T>
inline std::pair<id_t,std::unique_ptr<T>> make_pair_ptr(id_t id, T* res) {
  return std::make_pair(id, std::unique_ptr<T>{res});
}

template<typename... ResT>
class Pool {
protected:
  template<typename T>
  using ResMap = std::unordered_map<id_t, T>;

  using LoadCallback = std::function<void()>;

public:
  Pool() = default;
  ~Pool() = default;

  Pool(Pool&&) = default;
  Pool& operator=(Pool&&) = default;

  Pool(const Pool&) = delete;
  Pool& operator=(const Pool&) = delete;

public:
  template<same_as_defined<ResT...> T>
  cref<T> get(id_t id) {
    return std::cref(std::get<ResMap<T>>(pool).at(id));
  }

  template<same_as_defined<ResT...> T>
  T* get_p(id_t id) {
    return &std::get<ResMap<T>>(pool).at(id);
  }

public:
  template<same_as_defined<ResT...> T>
  void direct_load(std::initializer_list<PathInfo> input_list) {
    for (const auto& res_info : input_list) {
      auto& loader = DataLoader::instance();
      auto data_ptr = loader.direct_load<T>(res_info);
      this->emplace<T>(res_info.id, data_ptr.get());
    }
  }

  template<same_as_defined<ResT...> T>
  void async_load(std::initializer_list<PathInfo> input_list, LoadCallback on_load) {
    load_counters.push_back(std::make_pair(input_list.size(), 0));
    auto* counter = &load_counters.back();

    for (const auto& res_info : input_list) {
      auto& loader = DataLoader::instance();

      loader.async_load<T>(res_info, [this, counter, on_load](auto id, auto data_ptr) {
        size_t res_total = counter->first;
        size_t& res_c = counter->second;
        this->emplace<T>(id, data_ptr.get());
        if (++res_c == res_total) {
          on_load();
        }
      });
    }
  }

protected:
  template<typename T>
  void emplace(id_t id, T::data_t* data) {
    auto& map = std::get<ResMap<T>>(pool);
    map.emplace(std::make_pair(id, T{data}));
  }

protected:
  std::tuple<std::unordered_map<id_t, ResT>...> pool;
  std::vector<std::pair<size_t, size_t>> load_counters;
};


} // namespace ntf::shogle::res
