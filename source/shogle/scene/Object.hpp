#pragma once

namespace ntf::shogle {

class Object {
public:
  Object() = default;

public:
  virtual ~Object() = default;

public:
  virtual void update() = 0;
};

} // namespace ntf::shogle
