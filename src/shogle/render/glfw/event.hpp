#pragma once

#include <functional>

#include <concepts>

namespace ntf::shogle::glfw {

template<typename... Args>
class event {
public:
  using callback_t = std::function<void(Args...)>;
public:
  template<typename callback>
  void set_callback(callback&& cb) {
    _event = std::forward<callback>(cb);
  }
  void operator()(Args... args) {
    if (_event) { _event(args...); }
  }
private:
  std::function<void(Args...)> _event;
};


} // namespace ntf::shogle::glfe
