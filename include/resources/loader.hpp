#pragma once

#include "shogle/util/singleton.hpp"
#include "shogle/util/threadpool.hpp"
#include "shogle/res/shader_data.hpp"
#include "shogle/res/texture_data.hpp"
#include "shogle/res/model_data.hpp"

namespace ntf::shogle::res {

typedef std::unordered_map<std::string, res::ShaderData*> ShaderMap;
typedef std::unordered_map<std::string, res::TextureData*> TextureMap;
typedef std::unordered_map<std::string, res::ModelData*> ModelMap;

class ResourceLoader : public Singleton<ResourceLoader> {
public:
  ResourceLoader();

  void enqueue_shader(const std::string& shader_name);
  void unload_shaders(void);

  void enqueue_texture(const std::string& texture_name, GLenum tex_dim = GL_TEXTURE_2D);
  void unload_textures(void);

  void enqueue_model(const std::string& model_name);
  void unload_models(void);
  
  bool is_loading(void) { return this->load_count > 0; }

  std::mutex count_mutex;
  size_t load_count;

  std::mutex shader_mutex;
  ShaderMap shaders;

  std::mutex texture_mutex;
  TextureMap textures;

  std::mutex model_mutex;
  ModelMap models;

private:
  util::ThreadPool pool;
};

}
