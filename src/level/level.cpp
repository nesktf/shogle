#include "level/level.hpp"

namespace ntf::shogle {

void Level::next_state(void) {
  Log::verbose("[Level] Triggered next state");
  switch (this->state) {
    case State::Loading:
      this->state = State::Loaded;
      this->on_load();
      break;
    case State::Loaded:
      this->state = State::Transition;
      this->on_transition();
      break;
    default:
      break;
  }
}

void Level::update(float dt) {
  if (this->state == State::Loading) {
    update_loading(dt);
  } else if (this->state == State::Loaded) {
    update_loaded(dt);
  }
}

void Level::draw(void) {
  for (auto& lvl_obj : objs) {
    if (!lvl_obj.second->enabled)
      continue;

    lvl_obj.second->obj->draw();
  }
}

} // namespace ntf::shogle
