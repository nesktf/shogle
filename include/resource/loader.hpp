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

struct PathInfo {
  id_t id;
  std::string path;
};

template<typename T>
using dataptr_t = std::unique_ptr<typename T::data_t>;

template<typename T>
using LoaderCallback = std::function<void(id_t,dataptr_t<T>)>;

class DataLoader : public Singleton<DataLoader> {
public:
  DataLoader() = default;
  ~DataLoader() = default;

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
  dataptr_t<T> direct_load(PathInfo info) {
    return std::make_unique<typename T::data_t>(info.path);
  }

  template<typename T>
  void async_load(PathInfo info, LoaderCallback<T> on_load) {
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

} // namespace ntf::shogle::res

