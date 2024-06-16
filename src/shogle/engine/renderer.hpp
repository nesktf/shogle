#pragma once

#include <shogle/core/types.hpp>

#include <functional>
#include <vector>

namespace ntf::shogle {

template<typename T>
class renderer {
public:
  class drawable_t {
  public:
    drawable_t() = default;

  public:
    virtual ~drawable_t() = default;
    drawable_t(drawable_t&&) = default;
    drawable_t(const drawable_t&) = default;
    drawable_t& operator=(drawable_t&&) = default;
    drawable_t& operator=(const drawable_t&) = default;

  public:
    virtual void draw(const T& cam) = 0;
  };

  using drawfun = std::function<void(const T&)>;
  struct drawfun_wrapper : public drawable_t {
    drawfun_wrapper(drawfun fun) :
      _fun(std::move(fun)) {}

    void draw(const T& cam) override { _fun(cam); }

    drawfun _fun;
  };

public:
  renderer() = default;

public:
  void draw(const T& cam);
  void add(uptr<drawable_t> drawable);
  void add(drawfun fun);
  void clear();

  template<typename U, typename... Args>
  void emplace(Args&&... args);

private:
  std::vector<uptr<drawable_t>> _new_drawables;
  std::vector<uptr<drawable_t>> _drawables;
};

} // namespace ntf::shogle::render

#ifndef RENDERER_INL_HPP
#include <shogle/engine/renderer.hpp>
#endif
