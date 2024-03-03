#include "core/scene.hpp"

namespace ntf::shogle {

Scene::Scene() {}

Scene::~Scene() {
  for (auto& obj : objs) {
    delete obj.second;
  }
}

void Scene::draw(float delta_time) {
  for (auto& obj : objs) {
    obj.second->draw(delta_time);
  }
}

}
