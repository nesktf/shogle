#pragma once

#include <shogle/core/threadpool.hpp>
#include <shogle/core/types.hpp>

#include <memory>
#include <functional>

namespace ntf::res {

class async_loader {
public:
  using resid_t = std::string;

  struct pathinfo_t {
    resid_t id;
    path_t path;
  };


  template<typename T>
  using resdata_t = T::loader_t;

  template<typename T>
  using loadfun_t = std::function<void(resid_t,resdata_t<T>)>;

  template<typename T>
  struct load_wrapper {
    load_wrapper(resdata_t<T> data, resid_t id, loadfun_t<T> on_load) :
      _data(std::move(data)), _id(id), _on_load(std::move(on_load)) {}
    resdata_t<T> _data;
    resid_t _id;
    loadfun_t<T> _on_load;
  };

  using reqcallback_t = std::function<void()>;

public:
  async_loader() = default;

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
  resdata_t<T> direct_load(pathinfo_t info) {
    return resdata_t<T>{info.path};
  }

  template<typename T>
  void async_load(pathinfo_t info, loadfun_t<T> on_load) {
    _threadpool.enqueue([this, info, on_load=std::move(on_load)]{
      auto* wrapper = make_ptr<load_wrapper<T>>(resdata_t<T>{info.path}, info.id, std::move(on_load)); // Hopefully won't leak???

      std::unique_lock<std::mutex> lock{_req_mtx};
      _req.emplace([wrapper]() { // why are lambdas only copyconstructible?????
        wrapper->_on_load(wrapper->_id, std::move(wrapper->_data));
        delete wrapper;
      });
    });
  }

private:
  std::mutex _req_mtx;
  std::queue<reqcallback_t> _req;
  ThreadPool _threadpool;
};

} // namespace ntf::res
