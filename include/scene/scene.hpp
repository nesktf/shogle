#pragma once

#include "scene/task.hpp"

#include <functional>
#include <memory>

namespace ntf {

struct BaseScene {
  BaseScene() = default;

  virtual ~BaseScene() = default;

  BaseScene(BaseScene&&) = default; 
  BaseScene& operator=(BaseScene&&) = default;

  BaseScene(const BaseScene&) = delete;
  BaseScene& operator=(const BaseScene&) = delete;

  virtual void update(float dt) = 0;
};

using sceneptr_t = uptr<BaseScene>; 
using SceneCreator = std::function<sceneptr_t(void)>;

template<typename TScene>
struct Scene : public BaseScene {
  static sceneptr_t create(void) {
    return std::make_unique<TScene>();
  }
};


} // namespace ntf
