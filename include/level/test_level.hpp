#pragma once

#include "level/level.hpp"
#include "resource/resource.hpp"

namespace ntf::shogle {

class TestLevel : public Level {
public:
  TestLevel();
  ~TestLevel() = default;

public:
  void on_load(void) override;

  void update_loading(float dt) override;
  void update_loaded(float dt) override;

public:
  static Level* create(void) { return new TestLevel(); }

private:
  res::ResPool<res::Texture> init_tex;

  res::ResPool<res::Shader> shaders;
  res::ResPool<res::Model> models;
};

} // namespace ntf::shogle
