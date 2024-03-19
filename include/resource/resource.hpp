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

struct ResPath {
  id_t id;
  std::string path;
};

template<typename T>
class ResPool {
public:
  ResPool() = default;
  ~ResPool() = default;

  ResPool(ResPool&&) = default;
  ResPool& operator=(ResPool&&) = default;
  
  ResPool(const ResPool&) = delete;
  ResPool& operator=(const ResPool&) = delete;

public:
  void emplace(id_t id, std::unique_ptr<typename T::data_t> data) {
    // Create resource with base resource data
    pool.emplace(std::make_pair(id, T{data.get()}));
    Log::verbose("[ResPool] Created resource (id: {})", id);
    if (++load_c == load_total && load_callback) {
      Log::debug("[ResPool] Triggered load callback");
      load_callback();
    }
    Log::verbose("[ResPool] Progress: {}%", 100*progress());
  }

  std::reference_wrapper<const T> get(id_t id) {
    return std::cref(pool[id]);
  }

  T* get_p(id_t id) {
    return &pool.at(id);
  }

  void add_request(ResPath path) {
    requests.emplace(path);
    load_total = requests.size();
    Log::debug("[ResPool] Enqueued request (id: {}, path: {})", path.id, path.path);
  }

  float progress(void) { return (float)load_c/(float)load_total; }

public: 
  std::function<void()> load_callback;

  std::queue<ResPath> requests;
  std::unordered_map<id_t, T> pool;

  size_t load_total {0}, load_c {0};
};

class ResLoader : public Singleton<ResLoader> {
public:
  ResLoader() = default;
  ~ResLoader() = default;

public:
  void do_requests(void) {
    while (!requests.empty()) {
      auto callback = std::move(requests.front());
      requests.pop();
      callback();
    }
  }

public:
  template<typename T>
  void async_request(ResPool<T>& res_pool) {
    auto& res_req = res_pool.requests;
    while (!res_req.empty()) {
      auto res = std::move(res_req.front());
      t_pool.enqueue([this, &res_pool, res] {
        // Generate base resource data (partial load)
        auto* data = new T::data_t{res.path}; // Hopefully won't leak???

        // OpenGL objects have to be initialized in the main thread,
        // so we set a callback to create them with do_requests in
        // the next frame update, on the main thread
        std::unique_lock<std::mutex> req_lock{req_mtx};
        requests.emplace([data, &res_pool, id=res.id] {
          res_pool.emplace(id, std::unique_ptr<typename T::data_t>{data});
        });
      });
      res_req.pop();
      Log::verbose("[ResLoader] Created async request (id: {})", res.id);
    }
  }

  template<typename T>
  void request(ResPool<T>& res_pool) {
    auto& res_req = res_pool.requests;
    while (!res_req.empty()) {
      auto res = std::move(res_req.front());
      res_pool.emplace(res.id, std::make_unique<typename T::data_t>(res.path));
      res_req.pop();
      Log::verbose("[ResLoader] Created request (id: {})", res.id);
    }
  }

private:
  std::mutex req_mtx;
  std::queue<std::function<void()>> requests;
  ThreadPool t_pool;
};

} // namespace ntf::shogle::res

