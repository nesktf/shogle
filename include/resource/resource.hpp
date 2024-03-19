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
  ResPool(std::function<void()> callback) :
    load_callback(callback) {};

  ~ResPool() = default;

  ResPool(ResPool&&) = default;
  ResPool& operator=(ResPool&&) = default;
  
  ResPool(const ResPool&) = delete;
  ResPool& operator=(const ResPool&) = delete;

public:
  void emplace(id_t id, std::unique_ptr<typename T::data_t> data) {
    // Create resource with base resource data
    pool.emplace(std::make_pair(id, T{data.get()}));
    if (++load_c == load_total) {
      load_callback();
    }
  }

  std::reference_wrapper<const T> get(id_t id) {
    return std::cref(pool[id]);
  }

  T* get_p(id_t id) {
    return &pool[id];
  }

  void add_request(ResPath path) {
    requests.emplace(path);
    load_total++;
  }

public:
  std::function<void()> load_callback;

  std::queue<ResPath> requests;
  std::unordered_map<id_t, T> pool;

  size_t load_total, load_c;
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
      t_pool.enqueue([this, &res_pool, res=std::move(res_req.front())] {
        // Generate base resource data (partial load)
        auto data = std::make_unique<typename T::data_t>(res.path);

        // OpenGL objects have to be initialized in the main thread,
        // so we set a callback to create them with do_requests in
        // the next frame update, on the main thread
        std::unique_lock<std::mutex> req_lock{req_mtx};
        requests.emplace([data=std::move(data), &res_pool, id=res.id] {
          res_pool.emplace(id, std::move(data));
        });
      });
      res_req.pop();
    }
  }

private:
  std::mutex req_mtx;
  std::queue<std::function<void()>> requests;
  ThreadPool t_pool;
};

} // namespace ntf::shogle::res

