#pragma once

#include "res/texture.hpp"

#include <assimp/texture.h>

#include <string>

namespace ntf {

// Material::data_t
class MaterialData {
public: // Resource data can be copied but i don't think is a good idea
  MaterialData(std::string path);
  ~MaterialData() = default;

  MaterialData(MaterialData&&) = default;
  MaterialData& operator=(MaterialData&&) = default;

  MaterialData(const MaterialData&) = delete;
  MaterialData& operator=(const MaterialData&) = delete;

public:
  Texture::data_t tex;
  aiTextureType type;
};

// Material
class Material {
public:
  using data_t = MaterialData;

public: // Resource data can't be copied
  Material(const Material::data_t* data);
  ~Material();

  Material(Material&&) = default;
  Material& operator=(Material&&) = default;

  Material(const Material&) = delete;
  Material& operator=(const Material&) = delete;

public:
  void bind_sampler(const Shader& shader, size_t tex_num, size_t tex_ind) const;

public:
  Texture texture;
  aiTextureType type;
  std::string sampler_basename;
};

} // namespace ntf
