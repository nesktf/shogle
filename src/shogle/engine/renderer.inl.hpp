#define RENDERER_INL_HPP
#include <shogle/engine/renderer.hpp>
#undef RENDERER_INL_HPP

namespace ntf::shogle {

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

}
