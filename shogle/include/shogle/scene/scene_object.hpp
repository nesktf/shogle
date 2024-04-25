#pragma once

namespace ntf {

class SceneObj {
protected:
  SceneObj() = default;

public:
  virtual ~SceneObj() = default;
  SceneObj(SceneObj&&) = default;
  SceneObj(const SceneObj&) = default;
  SceneObj& operator=(SceneObj&&) = default;
  SceneObj& operator=(const SceneObj&) = default;

public:
  virtual void update(float dt) = 0;
};

} // namespace ntf
