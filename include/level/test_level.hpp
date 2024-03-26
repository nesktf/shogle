#pragma once

#include "level/level.hpp"

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
  res::Pool<res::Texture, res::Shader, res::Model> pool;

};

} // namespace ntf::shogle
