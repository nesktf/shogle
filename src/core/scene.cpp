#include "core/scene.hpp"

namespace ntf::shogle {

void Scene::ResourceProvider::setup_modeldata(ResourceMap&& map) {
  this->res_c += map.size();
}

void Scene::ResourceProvider::setup_texturedata(ResourceMap&& map) {
  this->res_c += map.size();
}

void Scene::ResourceProvider::setup_shaderdata(ResourceMap&& map) {
  this->res_c += map.size();
}

Scene::~Scene() {
  delete state;
}

Scene::Scene(Scene::State* load_state, Scene::ResourceProvider* provider) {
  this->state = load_state;
  this->provider = provider;
}

void Scene::update(float delta_time) {
  if (provider && provider->is_loaded()) {
    provider->on_load(this);
    delete provider;
    provider = nullptr;
  }

  state->on_update(delta_time, this);
}

void Scene::draw(void) {
  state->on_draw(this);
}

void Scene::set_state(Scene::State* new_state) {
}

}
