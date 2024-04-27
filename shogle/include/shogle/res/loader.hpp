#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/core/threadpool.hpp>
#include <shogle/core/types.hpp>

#include <memory>
#include <functional>

namespace ntf::res {

class loader : public Singleton<loader> {
public:
  using resid_t = std::string;

  struct pathinfo_t {
    resid_t id;
    path_t path;
  };

  template<typename T>
  using resdata_t = T::data_t;

  template<typename T>
  using loadfun_t = std::function<void(resid_t,uptr<resdata_t<T>>)>;

  using reqcallback_t = std::function<void()>;

public:
  loader() = default;

public:
  inline void do_requests(void) {
    while (!_req.empty()) {
      auto _request_callback = std::move(_req.front());
      _req.pop();
      _request_callback();
    }
  }

public:
  template<typename T>
  uptr<resdata_t<T>> direct_load(pathinfo_t info) {
    return make_uptr<resdata_t<T>>(info.path);
  }

  template<typename T>
  void async_load(pathinfo_t info, loadfun_t<T> on_load) {
    _threadpool.enqueue([this, info, on_load=std::move(on_load)]{
      auto* data = make_ptr<resdata_t<T>>(info.path); // Hopefully won't leak???

      std::unique_lock<std::mutex> lock{_req_mtx};
      _req.emplace([data, id=info.id, on_load=std::move(on_load)]{
        on_load(id, uptr<resdata_t<T>>{data});
      });
    });
  }

private:
  std::mutex _req_mtx;
  std::queue<reqcallback_t> _req;
  ThreadPool _threadpool;
};

} // namespace ntf::res
