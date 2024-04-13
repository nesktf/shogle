#pragma once

#include "core/singleton.hpp"
#include "core/threadpool.hpp"
#include "core/log.hpp"
#include "core/types.hpp"

#include <memory>
#include <functional>

namespace ntf {

using id_t = std::string;

struct pathinfo_t {
  id_t id;
  std::string path;
};

template<typename T>
using dataptr_t = uptr<typename T::data_t>;

template<typename T>
using LoaderCallback = std::function<void(id_t,dataptr_t<T>)>;

class ResLoader : public Singleton<ResLoader> {
public:
  ResLoader() = default;

public:
  inline void do_requests(void) {
    while (!requests.empty()) {
      auto callback = std::move(requests.front());
      requests.pop();
      callback();
    }
  }

public:
  template<typename T>
  dataptr_t<T> direct_load(pathinfo_t info) {
    return std::make_unique<typename T::data_t>(info.path);
  }

  template<typename T>
  void async_load(pathinfo_t info, LoaderCallback<T> on_load) {
    t_pool.enqueue([this, info, on_load=std::move(on_load)]{
      auto* data = new T::data_t{info.path}; // Hopefully won't leak???

      std::unique_lock<std::mutex> lock{req_mtx};
      requests.emplace([data, id=info.id, on_load=std::move(on_load)]{
        on_load(id, dataptr_t<T>{data});
      });
    });
  }

private:
  std::mutex req_mtx;
  std::queue<std::function<void()>> requests;
  ThreadPool t_pool;
};

} // namespace ntf
