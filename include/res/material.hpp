#pragma once

#include "res/texture.hpp"

#include <assimp/texture.h>
#include <assimp/scene.h>

#include <string>

namespace ntf {

enum class MaterialType {
  Diffuse,
  Specular
};

// Material::data_t
class MaterialData {
public:
  struct args_t {
    std::string path;
    aiTextureType type;
  };

public:
  MaterialData(args_t args);

public:
  Texture::data_t tex_data;
  std::string path;
  aiTextureType type;
};

// Material
class Material : public Texture {
public:
  using args_t = MaterialData::args_t;
  using data_t = MaterialData;

public:
  Material(const Material::data_t* data);

public:
  void bind_uniform(const Shader* shader, size_t tex_num, size_t tex_ind) const;

public:
  MaterialType type(void) const { return _type; }

private:
  MaterialType _type {MaterialType::Diffuse};
  std::string _uniform_basename {"material.diffuse"};
};

} // namespace ntf
