#include <shogle/res/material.hpp>

namespace ntf {

MaterialData::MaterialData(args_t args) :
  tex_data(args.path),
  path(args.path),
  type(args.type) {}

Material::Material(const Material::data_t* data) :
  Texture(&data->tex_data) {

  switch (data->type) {
    case aiTextureType_DIFFUSE: {
      _uniform_basename = "material.diffuse";
      _type = MaterialType::Diffuse;
      break;
    }
    case aiTextureType_SPECULAR: {
      _uniform_basename = "material.specular";
      _type = MaterialType::Specular;
      break;
    }
    default: {
      _uniform_basename = "material.diffuse";
      _type = MaterialType::Diffuse;
      break;
    }
  }
}

void Material::bind_uniform(const Shader* shader, size_t tex_num, size_t tex_ind) const {
  std::string uniform_name {_uniform_basename + std::to_string(tex_num) };
  glBindTexture(_dim, _tex);
  glActiveTexture(GL_TEXTURE0 + tex_ind);
  shader->unif_int(uniform_name.c_str(), tex_ind);
  shader->unif_float("material.col_shiny", 1.0f); // TODO: don't use fixed value
}

} // namespace ntf
