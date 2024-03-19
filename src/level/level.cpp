#include "level/level.hpp"

namespace ntf::shogle {

void Level::update(float dt) {

  switch (state) {

  };

  for (auto& lvl_obj : objs) {
    lvl_obj->update(dt);
    lvl_obj->obj->draw();
  }
}


} // namespace ntf::shogle
