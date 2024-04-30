#pragma once

#include <shogle/core/types.hpp>
#include <shogle/core/singleton.hpp>

#include <shogle/input/backends/glfw.hpp>

#include <functional>

namespace ntf::input {

using key = input::glfw; // for covenience

class KeyListener : public Singleton<KeyListener> {
private:
  using backend = input::glfw;

public:
  using listenerfun_t = std::function<void()>;
  using listenerid_t = uint;
  using keys = backend::key;
  using key_action = backend::key_action;

  struct key_listener {
    listenerfun_t listener;
    listenerid_t id;
    keys key;
    key_action action;
  };

public:
  KeyListener() = default;

public:
  void init(void);

public:
  listenerid_t register_listener(keys key, key_action action, listenerfun_t listener);
  bool unregister_listener(listenerid_t listener);

public:
  inline bool is_key_pressed(keys key) {
    return backend::instance().is_pressed(key);
  }

public:
  size_t _event_count {0};
  std::vector<key_listener> _listeners;
};

} // namespace ntf::input
