#pragma once

#include <shogle/render/common.hpp>

#include <shogle/assets/texture.hpp>

namespace ntf {

template<typename Texture>
struct mesh_data {
  using texture_data_type = texture_data<Texture>;

  struct vertex {
    using att_coord    = shader_attribute<0, vec3>;
    using att_normal   = shader_attribute<1, vec3>;
    using att_texcoord = shader_attribute<2, vec2>;

    vec3 coord;
    vec3 normal;
    vec2 tex_coord;
  };

  std::string name;
  std::vector<vertex> vertices;
  std::vector<uint> indices;
  pair_vector<material_type, texture_data_type> materials;
};


template<typename Mesh, typename Texture>
class model {
public:
  using texture_type = Texture;
  using mesh_type = Mesh;

  using mesh_data_type = mesh_data<Texture>;
  struct data_type;

  class textured_mesh;

public:
  model() = default;
  model(std::vector<mesh_data_type> meshes);

public:
  const textured_mesh& operator[](std::string_view name) const { return _meshes.at(name.data()); }
  const textured_mesh& at(std::string_view name) const { return _meshes.at(name.data()); }

  auto cbegin() const { return _meshes.cbegin(); }
  auto cend() const { return _meshes.cend(); }
  auto begin() { return _meshes.begin(); }
  auto end() { return _meshes.end(); }

  size_t size() const { return _meshes.size(); }

private:
  strmap<textured_mesh> _meshes;
};


template<typename Mesh, typename Texture>
struct model<Mesh, Texture>::data_type {
public:
  using texture_data_type = texture_data<Texture>;
  using mesh_data_type = mesh_data<Texture>;

public:
  struct loader {
    model operator()(data_type data) {
      return model{std::move(data.meshes)};
    }
    model operator()(std::string_view path, tex_filter filter, tex_wrap wrap) {
      return (*this)(data_type{path, filter, wrap});
    }
  };

public:
  data_type(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  std::vector<mesh_data_type> meshes;
};


template<typename Mesh, typename Texture>
class model<Mesh, Texture>::textured_mesh {
public:
  using texture_type = Texture;
  using mesh_type = Mesh;

public:
  textured_mesh(mesh_type mesh, std::unordered_map<material_type, texture_type> materials) :
    _mesh(std::move(mesh)), _materials(std::move(materials)) {}

public:
  const mesh_type& mesh() const { return _mesh; }
  const texture_type& operator[](material_type material) const { return _materials.at(material); }

  auto cbegin() const { return _materials.cbegin(); }
  auto cend() const { return _materials.cend(); }
  auto begin() { return _materials.begin(); }
  auto end() { return _materials.end(); }

  size_t size() const { return _materials.size(); }

private:
  mesh_type _mesh;
  std::unordered_map<material_type, texture_type> _materials;
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_MODEL_INL
#include <shogle/assets/model.inl>
#endif
