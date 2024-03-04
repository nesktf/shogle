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
    virtual ~State() {}

    virtual void on_update(float delta_time) = 0;
    virtual void on_draw(void) = 0;
    virtual void on_transition(void) = 0;
  };
  friend class State;

  virtual ~Scene();

  virtual void update(float delta_time) = 0;
  virtual void draw(void) = 0;

protected:
  void setup_modeldata(ResourceMap&& map);
  void setup_texturedata(ResourceMap&& map);
  void setup_shaderdata(ResourceMap&& map);

  void init_models(void);
  void init_textures(void);
  void init_shaders(void);

  std::unordered_map<std::string, std::unique_ptr<Model>> models;
  std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
  std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;

  bool is_loaded(void) const;

  void set_state(Scene::State* new_state);

  Scene::State* state;

private:
  std::unordered_map<std::string, ModelData*> model_data;
  std::unordered_map<std::string, TextureData*> texture_data;
  std::unordered_map<std::string, ShaderData*> shader_data;
  bool model_flag, texture_flag, shader_flag;
};

}
