#pragma once

#include <shogle/core/types.hpp>

#include <shogle/scene/camera.hpp>
#include <shogle/scene/object.hpp>

#include <functional>
#include <vector>

namespace ntf::shogle::render {

template<typename T>
class drawable {
public:
  drawable() = default;

public:
  virtual ~drawable() = default;
  drawable(drawable&&) = default;
  drawable(const drawable&) = default;
  drawable& operator=(drawable&&) = default;
  drawable& operator=(const drawable&) = default;

public:
  virtual void draw(const T& cam) = 0;
};

template<typename T>
class renderer {
public:
  using drawable_t = drawable<T>;

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

  template<typename U, typename... Args>
  void emplace(Args&&... args);

  void clear();

private:
  std::vector<uptr<drawable_t>> _new_drawables;
  std::vector<uptr<drawable_t>> _drawables;
};


using renderer2d = renderer<scene::camera2d>;
using drawable2d = renderer2d::drawable_t;
using renderer3d = renderer<scene::camera3d>;
using drawable3d = renderer3d::drawable_t;


template<typename T>
void renderer<T>::draw(const T& cam) {
  for (auto& drawable : _new_drawables) {
    _drawables.push_back(std::move(drawable));
  }
  _new_drawables.clear();

  for (auto& drawable : _drawables) {
    drawable->draw(cam);
  }
}

template<typename T>
void renderer<T>::add(uptr<drawable_t> drawable) {
  _new_drawables.emplace_back(std::move(drawable));
}

template<typename T>
void renderer<T>::add(drawfun fun) {
  _new_drawables.emplace_back(make_uptr<drawfun_wrapper>(std::move(fun)));
}

template<typename T>
template<typename U, typename... Args>
void renderer<T>::emplace(Args&&... args) {
  _new_drawables.emplace_back(make_uptr<U>(std::forward<Args>(args)...));
}

template<typename T>
void renderer<T>::clear() { _new_drawables.clear(); _drawables.clear(); }

} // namespace ntf::shogle::render
