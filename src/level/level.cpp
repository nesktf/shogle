#include "level/level.hpp"

namespace ntf::shogle {

void Level::next_state(void) {
  switch (this->state) {
    case State::Loading:
      Log::debug("[Level] State change (Loading -> Loaded)");
      this->state = State::Loaded;
      this->on_load();
      break;
    case State::Loaded:
      Log::debug("[Level] State change (Loaded -> Transition)");
      this->state = State::Transition;
      this->on_transition();
      break;
    default:
      break;
  }
}

void Level::update(float dt) {
  for (auto& obj : model_obj) {
    obj.second->update(dt);
  }
  for (auto& obj : sprite_obj) {
    obj.second->update(dt);
  }
}

void Level::draw(void) {
  for (auto& obj : model_obj) {
    if (!obj.second->enable) {
      continue;
    }
    obj.second->draw();
  }
  for (auto& obj : sprite_obj) {
    if (!obj.second->enable) {
      continue;
    }
    obj.second->draw();
  }
}

} // namespace ntf::shogle
