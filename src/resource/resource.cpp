#include "shogle/core/loader.hpp"
#include "shogle/core/logger.hpp"
#include "shogle/core/const.hpp"

namespace ntf::shogle {

ResourceLoader::ResourceLoader() {
  this->load_count = 0;
  logger::debug("(ResourceLoader) Initialized");
}

void ResourceLoader::enqueue_shader(const std::string& shader_name) {
  logger::debug("(ResourceLoader) Enqueued shader: {}", shader_name);
  std::unique_lock<std::mutex> count_lock(count_mutex);
  ++this->load_count;
  this->pool.enqueue([this, shader_name] {
    auto shader_path = consts::shader_folder + shader_name + ".glsl";
    ShaderData* data = new ShaderData(shader_path.c_str());

    std::unique_lock<std::mutex> shader_lock(shader_mutex);
    shaders[shader_name] = data;

    std::unique_lock<std::mutex> count_lock(count_mutex);
    --this->load_count;
    logger::debug("(ResourceLoader) Loaded shader: {}", shader_name);
  });
}

void ResourceLoader::unload_shaders(void) {
  for (auto& shader : shaders) {
    delete shader.second;
  }
  shaders.clear();
}

void ResourceLoader::enqueue_texture(const std::string& texture_name, GLenum tex_dim) {
  logger::debug("(ResourceLoader) Enqueued texture: {}", texture_name);
  std::unique_lock<std::mutex> count_lock(count_mutex);
  ++this->load_count;
  this->pool.enqueue([this, texture_name, tex_dim] {
    auto tex_path = consts::texture_folder + texture_name;
    auto* data = new TextureData(tex_path.c_str(), tex_dim, aiTextureType_DIFFUSE);

    std::unique_lock<std::mutex> tex_lock(texture_mutex);
    textures[texture_name] = data;

    std::unique_lock<std::mutex> count_lock(count_mutex);
    --this->load_count;
    logger::debug("(ResourceLoader) Loaded texture: {}", texture_name);
  });
}

void ResourceLoader::unload_textures(void) {
  for (auto& texture : textures) {
    delete texture.second;
  }
  textures.clear();
}

void ResourceLoader::enqueue_model(const std::string& model_name) {
  logger::debug("(ResourceLoader) Enqueued model: {}", model_name);
  std::unique_lock<std::mutex> count_lock(count_mutex);
  ++this->load_count;
  this->pool.enqueue([this, model_name] {
    auto model_path = consts::model_folder + model_name + "/" + model_name + ".obj";
    auto* data = new ModelData(model_path.c_str());

    TextureMap mesh_textures;
    for (auto& mesh : data->meshes) {
      for (auto& tex : mesh.tex) {
        auto tex_name = model_name + "/" + tex.name;
        auto tex_path = consts::model_folder + tex_name;
        mesh_textures[tex_name] = new TextureData(tex_path.c_str(), GL_TEXTURE_2D, tex.type);
      }
    }

    std::unique_lock<std::mutex> model_lock(model_mutex);
    models[model_name] = data;

    std::unique_lock<std::mutex> texture_lock(texture_mutex);
    textures.insert(mesh_textures.begin(), mesh_textures.end());

    std::unique_lock<std::mutex> count_lock(count_mutex);
    --this->load_count;
    logger::debug("(ResourceLoader) Loaded model: {}", model_name);
  });
}

void ResourceLoader::unload_models(void) {
  for (auto& model : models) {
    delete model.second;
  }
  models.clear();
}

}
