#pragma once

#include <shogle/core/types.hpp>

#include <functional>
#include <memory>

namespace ntf {

struct Scene {
  struct Object {
    Object() = default;

    virtual ~Object() = default;
    Object(Object&&) = default;
    Object(const Object&) = default;
    Object& operator=(Object&&) = default;
    Object& operator=(const Object&) = default;

    virtual void update(float dt) = 0;
  };

  Scene() = default;

  virtual ~Scene() = default;
  Scene(Scene&&) = default; 
  Scene(const Scene&) = delete;
  Scene& operator=(Scene&&) = default;
  Scene& operator=(const Scene&) = delete;

  virtual void update(float dt) = 0;
  virtual void draw_ui(void) {}
};

using sceneptr_t = uptr<Scene>; 
using SceneCreator = std::function<sceneptr_t(void)>;

} // namespace ntf
