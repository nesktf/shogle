#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <functional>
#include <algorithm>

namespace ntf {

template<typename T, typename... Args>
class listener {
public:
  using data_t = T;

  using id_t = uint;
  using fun_t = std::function<void(Args...)>;
  using upd_fun_t = std::function<bool(data_t&)>;

  struct listener_t {
    id_t id;
    fun_t fun;
    data_t data;
  };

public:
  listener(upd_fun_t condition = [](auto&) { return true; }) :
    _condition(condition) {}

public:
  void update(Args&&... args) {
    for (auto& listener : _listeners) {
      if (_condition(listener.data)) {
        listener.fun(args...);
      }
    }
  }

  template<typename... TArgs>
  id_t register_observer(fun_t fun, TArgs&&... data_args) {
    _listeners.emplace_back(
      ++_listener_count, 
      std::move(fun), 
      data_t{std::forward<TArgs>(data_args)...}
    );
    return _listener_count;
  }

  bool unregister_observer(id_t id) {
    auto match = [id](auto& listener) { return listener.id == id; };
    auto it = std::find_if(_listeners.begin(), _listeners.end(), match);
    if (it != _listeners.end()) {
      _listeners.erase(it);
      return true;
    }
    return false;
  }

private:
  id_t _listener_count {0};
  upd_fun_t _condition;
  std::vector<listener_t> _listeners;
};

} // namespace ntf
