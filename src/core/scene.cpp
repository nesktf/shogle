#include "core/scene.hpp"

namespace ntf::shogle {

Scene::~Scene() {
}

void Scene::setup_modeldata(ResourceMap&& map) {

}

void Scene::setup_texturedata(ResourceMap&& map) {

}

void Scene::setup_shaderdata(ResourceMap&& map) {

}

void Scene::init_models(void) {

}

void Scene::init_textures(void) {

}

void Scene::init_shaders(void) {

}

bool Scene::is_loaded(void) const {
  return model_flag && texture_flag && shader_flag;
}

}
