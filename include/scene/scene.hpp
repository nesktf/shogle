#pragma once

#include <functional>
#include <memory>

namespace ntf {

struct Scene {
  Scene() = default;
  virtual ~Scene() = default;

  Scene(Scene&&) = default; 
  Scene& operator=(Scene&&) = default;

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  virtual void update(float dt) = 0;
};

using SceneCreator = std::function<std::unique_ptr<Scene>(void)>;

} // namespace ntf
