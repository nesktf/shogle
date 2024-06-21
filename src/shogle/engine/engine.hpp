#pragma once

#include <shogle/render/render.hpp>
#include <shogle/render/glfw/window.hpp>
#include <shogle/render/imgui/imgui.hpp>

#include <shogle/engine/event_traits.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

class engine {
private:
  struct destructor_logger {
    ~destructor_logger() { log::debug("[shogle::engine] Engine destroyed "); }
  };

public:
  engine(size_t w, size_t h, std::string title);

public:
  void start();

public:
  /**
   * @brief Set the loop draw event callback (required). Called at the end of the loop.
   *
   * @param fun Draw function void()
   * @return this
   */
  template<drawfun T>
  engine& set_draw_event(T&& fun);

  /**
   * @brief Set the loop update event callback (required). Called before the draw callback.
   *
   * @param fun Update function void(float)
   * @return this
   */
  template<updatefun T>
  engine& set_update_event(T&& fun);

  /**
   * @brief Set viewport event callback (optional)
   *
   * @param fun Viewport function void(size_t,size_t)
   * @return this
   */
  template<viewportfun T>
  engine& set_viewport_event(T&& fun);

  /**
   * @brief Set the key event callback (optional)
   *
   * @param fun Key function void(keycode,scancode,keystate,keymod)
   * @return this
   */
  template<keyfun T>
  engine& set_key_event(T&& fun);

  /**
   * @brief Set the cursor event callback (optional)
   *
   * @param fun Cursor function void(double,double)
   * @return this
   */
  template<cursorfun T>
  engine& set_cursor_event(T&& fun);

  /**
   * @brief Set the scroll event callback (optional)
   *
   * @param fun Cursor function void(double,double)
   * @return this
   */
  template<scrollfun T>
  engine& set_scroll_event(T&& fun);

public:
  class window& window() { return _window; }

public:
  // Should live in main(), can't move or copy :p
  engine(engine&&) = delete;
  engine(const engine&) = delete;
  engine& operator=(engine&&) = delete;
  engine& operator=(const engine&) = delete;

private:
  destructor_logger _deslog;
  class window _window;
  imgui_handle _imgui;

  std::function<void()> _draw_event;
  std::function<void(float)> _update_event;
};

} // namespace ntf::shogle

#ifndef ENGINE_INL_HPP
#include <shogle/engine/engine.inl.hpp>
#endif

