#pragma once

#include <shogle/core/singleton.hpp>

#include <shogle/scene/scene.hpp>

namespace ntf {

class SceneManager : public Singleton<SceneManager> {

  void draw_scene(float dt);

  sceneptr_t scene;
};

} // namespace ntf
