#pragma once

#include "task/task.hpp"

#include "log.hpp"
#include "traits.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

template<typename TDrawable>
requires(is_drawable<TDrawable>)
class GameObject {
public: 
  template<typename... Args>
  GameObject(Args&&... args) :
    obj(TDrawable{std::forward<Args>(args)...}) {}

  ~GameObject() = default;

  GameObject(GameObject&&) = default;
  GameObject& operator=(GameObject&&) = default;

  GameObject(const GameObject&) = default;
  GameObject& operator=(const GameObject&) = default;

public:
  inline void set_transform(TransformData transf) {
    this->transform = transf;
    obj.model_m = TDrawable::model_transform(transf);
  }

  inline TransformData get_transform(void) {
    return this->transform;
  }

  inline void draw(void) { if (enable) obj.draw(); }

public:
  TDrawable obj;
  TransformData transform {};
  bool enable {true};
};

} // namespace ntf::shogle
