#pragma once

#include <shogle/core/types.hpp>

#include <functional>
#include <memory>

namespace ntf {

struct shogle_state;

class scene {
public:
  class object {
  public:
    object() = default;

  public:
    virtual ~object() = default;
    object(object&&) = default;
    object(const object&) = default;
    object& operator=(object&&) = default;
    object& operator=(const object&) = default;

  public:
    virtual void update(float dt) = 0;
  };

  class drawable : public object {
  public:
    virtual void draw(void) = 0;
  };

public:
  scene() = default;

public:
  virtual ~scene() = default;
  scene(scene&&) = default; 
  scene(const scene&) = delete;
  scene& operator=(scene&&) = default;
  scene& operator=(const scene&) = delete;

public:
  virtual void on_create(shogle_state& state) = 0;
  virtual void update(shogle_state& state, float dt) = 0;
  virtual void draw(shogle_state& state) = 0;
};

using scene_creator_t = uptr<scene> (*)(void);

} // namespace ntf
