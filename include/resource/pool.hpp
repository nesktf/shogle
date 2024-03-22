#pragma once

#include "resource/loader.hpp"

namespace ntf::shogle::res {

using PoolLoadCallback = std::function<void()>;

template<typename T>
class BasePool {
protected:
  BasePool() = default;
public:
  virtual ~BasePool() = default;

  BasePool(BasePool&&) = default;
  BasePool& operator=(BasePool&&) = default;

  BasePool(const BasePool&) = delete;
  BasePool& operator=(const BasePool&) = delete;

public:
  std::reference_wrapper<const T> get(id_t id) {
    return std::cref(pool.at(id));
  }
  T* get_p(id_t id) {
    return &pool.at(id);
  }

protected:
  void emplace(id_t id, T::data_t* data) {
    pool.emplace(std::make_pair(id, T{data}));
  }

protected:
  std::unordered_map<id_t, T> pool;
};

template<typename T>
class Pool : public BasePool<T> {
public:
  Pool(std::initializer_list<PathInfo> input_list) {
    for (const auto& res_info : input_list) {
      auto& loader = DataLoader::instance();
      auto data_ptr = loader.direct_load<T>(res_info);
      this->emplace(res_info.id, data_ptr.get());
    }
  }
};

template<typename T>
class AsyncPool : public BasePool<T> {
public:
  AsyncPool(std::initializer_list<PathInfo> input_list, PoolLoadCallback cb) : 
    on_load(cb),
    res_total(input_list.size()),
    res_c(0) {
    for (const auto& res_info : input_list) {
      auto& loader = DataLoader::instance();
      loader.async_load<T>(res_info, [this](auto id, auto data_ptr) {
        this->emplace(id, data_ptr.get());
        if (++res_c == res_total) {
          on_load();
        }
      });
    }
  };

public:
  float progress(void) { return (float)res_c/(float)res_total; }

private:
  PoolLoadCallback on_load;
  size_t res_total, res_c;
};

} // namespace ntf::shogle::res
