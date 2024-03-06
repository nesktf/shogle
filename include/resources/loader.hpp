#pragma once

#include "core/event.hpp"
#include "util/singleton.hpp"
#include "util/threadpool.hpp"
#include "resources/shader_data.hpp"
#include "resources/texture_data.hpp"
#include "resources/model_data.hpp"
#include "core/model.hpp"
#include <memory>
#include <functional>

namespace ntf::shogle {

typedef std::unordered_map<std::string, std::string> ResourceList;

template <typename T>
using ResourceMap = std::unordered_map<std::string, T>;

class ResourceLoader : public Singleton<ResourceLoader> {
public:
  ResourceLoader();
  ~ResourceLoader() = default;

public:
  void run_callbacks(void) {
    while (true) {
      if (res_callbacks.empty())
        return;
      auto callback = std::move(res_callbacks.front());
      callback();
      res_callbacks.pop();
    }
  }

public:
  template <typename T>
  void res_req(const ResourceList& list, std::function<void(std::unique_ptr<T>, std::string)> callback) {
    for (const auto& item : list) {
      this->pool.enqueue([this, callback, item] {
        auto data = std::make_unique<T>(item.second.c_str());
        std::unique_lock<std::mutex> cb_lock{this->callback_mutex};
        this->res_callbacks.emplace([data=std::move(data), callback=std::move(callback), item]() mutable {
          callback(std::move(data), item.first);
        });
      });
    }
  }
  
private:
  std::mutex callback_mutex;
  std::queue<std::function<void()>> res_callbacks;
  ThreadPool pool;
  size_t load_count;
};


}
