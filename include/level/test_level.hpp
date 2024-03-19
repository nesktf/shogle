#pragma once

#include "level/level.hpp"

namespace ntf::shogle {

class TestLevel : public Level {

public:
  static Level* create(void) { return new TestLevel(); }
};

} // namespace ntf::shogle
