#pragma once

#include <shogle/render/glfw.hpp>

#include <functional>

namespace ntf {

class input_event {
private:
  using glfw = render::glfw;

public:
  using event_t = std::function<void()>;
  using id_t = uint;

  struct key_event {
    id_t id;
    event_t fun;
    glfw::key key;
    glfw::key_action key_action;
  };

public:
  input_event(glfw::window win);

public:
  id_t subscribe(glfw::key key, glfw::key_action key_action, event_t event);
  bool unsuscribe(id_t id);
  void fire(glfw::key key, glfw::key_action key_action);
  void clear(void);

public:
  inline bool is_key_pressed(glfw::key key) {
    return glfw::is_key_pressed(_win, key);
  }

public:
  std::vector<key_event> _key_events;

private:
  id_t _event_count {0};
  glfw::window _win;
};


} // namespace ntf
