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
  res::Pool<res::Texture> tex_pool;
  res::Pool<res::Shader> sha_pool;
  res::AsyncPool<res::Model> mod_pool;

};

} // namespace ntf::shogle
