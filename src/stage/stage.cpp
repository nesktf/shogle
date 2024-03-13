#include "core/scene.hpp"
#include <algorithm>

namespace ntf::shogle {

void Scene::update(float delta_time) {
  for (auto& manip : new_obj_manip) {
    obj_manip.push_back(std::move(manip));
  }
  new_obj_manip.clear();

  for (auto& manip : obj_manip) {
    manip->update(delta_time);
    // do things, add new manipulators...
  }
  obj_manip.erase(std::remove_if(obj_manip.begin(), obj_manip.end(), [](const std::unique_ptr<Manipulator>& p) {
    return p->is_complete;
  }));

  for (auto& obj : objs) {
    obj.update(delta_time);
  }
}

void Scene::draw() const {
  for (auto& obj : objs) {
    obj.draw();
  }
}

} // namespace ntf::shogle
