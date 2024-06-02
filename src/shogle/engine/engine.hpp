#pragma once

#include <shogle/render/glfw/window.hpp>
#include <shogle/render/imgui/imgui.hpp>

#include <shogle/engine/event_traits.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

/**
 * @class engine
 * @brief Simple main loop wrapper. Handles a GLFW window and an imgui context.
 *        You should call your gl functions only after this is created.
 */
class engine {
private:
  struct destructor_logger {
    ~destructor_logger() { log::debug("[shogle::engine] Engine destroyed "); }
  };

public:
  /**
   * @brief Create a window with sizes w*h and a title
   *
   * @param w Window width
   * @param h Window height
   * @param title Window title
   */
  engine(size_t w, size_t h, std::string title);

public:
  /**
   * @brief Start the render loop. draw_event and update_event must be set before calling this
   */
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
  /**
   * @brief GLFW window getter
   *
   * @return GLFW window ref
   */
  glfw::window& window() { return _window; }

public:
  // Should live in main(), can't move or copy :p
  engine(engine&&) = delete;
  engine(const engine&) = delete;
  engine& operator=(engine&&) = delete;
  engine& operator=(const engine&) = delete;

private:
  destructor_logger _deslog;
  glfw::window _window;
  imgui::renderer _imgui;

  std::function<void()> _draw_event;
  std::function<void(float)> _update_event;
};

} // namespace ntf::shogle

#ifndef ENGINE_INL_HPP
#include <shogle/engine/engine.inl.hpp>
#endif

