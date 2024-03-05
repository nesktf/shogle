#pragma once

#include "core/game_object.hpp"
#include "resources/model_data.hpp"
#include "resources/texture_data.hpp"
#include "resources/shader_data.hpp"

#include "core/model.hpp"
#include "core/texture.hpp"
#include "core/shader.hpp"

#include <unordered_map>
#include <memory>

namespace ntf::shogle {

typedef std::unordered_map<std::string, std::string> ResourceMap;
typedef std::unordered_map<std::string, std::unique_ptr<Model>> ModelMap;
typedef std::unordered_map<std::string, std::unique_ptr<Texture>> TextureMap;
typedef std::unordered_map<std::string, std::unique_ptr<Shader>> ShaderMap;


class Scene {
public:
  class State {
  public:
    virtual ~State() = default;

    virtual void on_update(float delta_time, Scene* scene) = 0;
    virtual void on_draw(Scene* scene) = 0;
  };

  class ResourceProvider {
  public:
    virtual ~ResourceProvider() = default;
    virtual void on_load(Scene* scene) = 0;

    bool is_loaded(void) { return this->load_c == this->res_c; }

  protected:
    void setup_modeldata(ResourceMap&& map);
    void setup_texturedata(ResourceMap&& map);
    void setup_shaderdata(ResourceMap&& map);

  private:
    std::unordered_map<std::string, std::unique_ptr<ModelData>> model_data;
    std::unordered_map<std::string, std::unique_ptr<TextureData>> texture_data;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shader_data;
    size_t load_c;
    size_t res_c;
  };

  virtual ~Scene();

  void update(float delta_time);
  void draw(void);

  void set_state(Scene::State* new_state);

  ModelMap models;
  TextureMap textures;
  ShaderMap shaders;

protected:
  Scene(Scene::State* load_state, Scene::ResourceProvider* provider);

  ResourceProvider* provider;
  Scene::State* state;

  // void init_models(void);
  // void init_textures(void);
  // void init_shaders(void);

  // std::unordered_map<std::string, std::unique_ptr<Model>> models;
  // std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
  // std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
};

}
