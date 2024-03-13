#pragma once

#include "singleton.hpp"
#include "threadpool.hpp"
#include "log.hpp"

#include "resource/shader.hpp"
#include "resource/texture.hpp"
#include "resource/model.hpp"

#include <memory>
#include <functional>

namespace ntf::shogle::res {

using id_t = std::string;

template<typename T>
using data_callback_t = std::function<void(std::unique_ptr<typename T::data_t,id_t>)>;

typedef std::unordered_map<id_t, std::string> ResourceList;

template<typename T>
using ResourceMap = std::unordered_map<std::string, T>;

class ResourceLoader : public Singleton<ResourceLoader> {
public:
  ResourceLoader();
  ~ResourceLoader() = default;

public:
  void do_requests(void) {
    while (!callbacks.empty()) {
      auto callback = std::move(callbacks.front());
      callback();
      callbacks.pop();
    }
  }

public:
  template<typename T>
  void request_resources(const ResourceList& list, std::function<void(std::unique_ptr<T>, std::string)> callback) {
    for (const auto& item : list) {
      this->pool.enqueue([this, callback, item] {
        auto data = std::make_unique<T>(item.second.c_str());
        std::unique_lock<std::mutex> cb_lock{this->callback_mutex};
        this->callbacks.emplace([data=std::move(data), callback=std::move(callback), name=item.second]() mutable {
          callback(std::move(data), name);
        });
      });
    }
  }

  template<typename T>
  void async_load(std::unordered_map<id_t, T>& res_pool, id_t id, data_callback_t<T> callback) {
  }
  
private:
  std::mutex callback_mutex;
  std::queue<std::function<void()>> callbacks;
  ThreadPool pool;
  size_t load_count;
};



template<typename T>
class ResourcePool {
public:
  ResourcePool() = default;
  ResourcePool(std::function<void()> callback) :
    load_callback(callback) {};

  ~ResourcePool() = default;

  ResourcePool(ResourcePool&&) = default;
  ResourcePool& operator=(ResourcePool&&) = default;
  
  ResourcePool(const ResourcePool&) = delete;
  ResourcePool& operator=(const ResourcePool&) = delete;

public:
  void emplace(std::unique_ptr<typename T::data_t> data, id_t id, bool global = false) {
    if (global) {
      global_pool.emplace(std::make_pair(id, T{data.get()}));
    } else {
      pool.emplace(std::make_pair(id, T{data.get()}));
    }
    if (++load_c == load_total) {
      load_callback();
    }
  }

  std::reference_wrapper<const T> get(id_t id) {
    if (global_pool.find(id) != global_pool.end()) {
      return std::cref(global_pool[id]);
    } else {
      return std::cref(pool[id]);
    }
  }

  void load(id_t id) {
    if (pool.find(id) != pool.end()) {
      return;
    }
    ++load_total;
    auto& loader {ResourceLoader::instance()};
  }

  void load_global(id_t id) {
    if (global_pool.find(id) != global_pool.end()) {
      return;
    }
    ++load_total;
    auto& loader {ResourceLoader::instance()};
    loader.async_load(global_pool, id, [&](auto data, auto id){
      this->emplace(std::move(data), id);
    });
  }

public:
  std::function<void()> load_callback;

private:
  std::unordered_map<id_t, T> pool;
  static std::unordered_map<id_t, T> global_pool;
  size_t load_total, load_c;
};

} // namespace ntf::shogle::res
