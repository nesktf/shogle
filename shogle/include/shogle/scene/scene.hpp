#pragma once

#include <shogle/scene/task.hpp>

#include <functional>
#include <memory>

namespace ntf {

struct BaseScene {
  BaseScene() = default;

  virtual ~BaseScene() = default;

  BaseScene(BaseScene&&) = default; 
  BaseScene(const BaseScene&) = delete;
  BaseScene& operator=(BaseScene&&) = default;
  BaseScene& operator=(const BaseScene&) = delete;

  virtual void update(float dt) = 0;
  virtual void ui_draw(void) {}
};

using sceneptr_t = uptr<BaseScene>; 
using SceneCreator = std::function<sceneptr_t(void)>;

template<typename T>
struct Scene : public BaseScene {
  static sceneptr_t create(void) {
    return std::make_unique<T>();
  }
};

template<typename T>
struct TaskedScene : public Scene<T>, public TaskManager<T>{};


} // namespace ntf
