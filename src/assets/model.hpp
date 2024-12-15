#pragma once

#include "./assets.hpp"
#include "./texture.hpp"
#include "../render/render.hpp"

namespace ntf {

using material_index = uint32_t;

template<typename Texture>
class material {
public:
  using texture_type = Texture;

  using iterator = std::vector<texture_type>::iterator;
  using const_iterator = std::vector<texture_type>::const_iterator;

public:
  material() = default;
  material(std::vector<texture_type> materials, 
           std::unordered_map<material_category, material_index> material_types);

public:
  const texture_type& operator[](material_index pos) const;
  texture_type& operator[](material_index pos);

  std::optional<material_index> find(material_category type);

public:
  std::size_t size() const { return _materials.size();}

  iterator begin() { return _materials.begin(); }
  const_iterator begin() const { return _materials.begin(); }
  const_iterator cbegin() const { return _materials.cbegin(); }

  iterator end() { return _materials.end(); }
  const_iterator end() const { return _materials.end(); }
  const_iterator cend() const { return _materials.cend(); }

private:
  std::vector<texture_type> _materials;
  std::unordered_map<material_category, std::size_t> _material_types;
};

template<typename Mesh, typename Texture>
class model {
public:
  using mesh_type = Mesh;
  using texture_type = Texture;

public:
  model() = default;

  model(std::vector<mesh_type> meshes, std::unordered_map<std::string, std::size_t> names,
        std::vector<material<texture_type>> materials);

private:
  std::vector<mesh_type> _meshes;
  std::unordered_map<std::string, std::size_t> _mesh_names;
  std::vector<material<texture_type>> _materials;
};


struct basic_vertex {
  using att0 = shader_attribute<0, vec3>;
  using att1 = shader_attribute<1, vec3>;
  using att2 = shader_attribute<2, vec2>;

  vec3 coord;
  vec3 normal;
  vec2 tex_coord;
};


template<typename Mesh, typename Texture, typename Vertex = basic_vertex>
class assimp_data_loader {
public:
  using resource_type = model<Mesh, Texture>;

private:
  using texture_type = Texture;
  using vertex_type = Vertex;

  struct basic_mesh_data {
    std::string name;
    std::vector<vertex_type> vertices;
    std::vector<uint> indices;
    std::vector<std::pair<material_category, basic_texture_data<texture_type>>> materials;
  };

public:
  using data_type = std::vector<basic_mesh_data>;

public:
  bool resource_load(std::string_view path);
  void resource_unload(bool overwrite);

  std::optional<resource_type> make_resource() const;

private:
  data_type _data;
};

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
  std::vector<std::pair<material_category, texture_data_type>> materials;
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
  std::unordered_map<std::string, textured_mesh> _meshes;
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
    model operator()(std::string path, tex_filter filter, tex_wrap wrap) {
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
  textured_mesh(mesh_type mesh, std::unordered_map<material_category, texture_type> materials) :
    _mesh(std::move(mesh)), _materials(std::move(materials)) {}

public:
  const mesh_type& mesh() const { return _mesh; }
  const texture_type& operator[](material_category material) const { return _materials.at(material); }

  auto cbegin() const { return _materials.cbegin(); }
  auto cend() const { return _materials.cend(); }
  auto begin() { return _materials.begin(); }
  auto end() { return _materials.end(); }

  size_t size() const { return _materials.size(); }

private:
  mesh_type _mesh;
  std::unordered_map<material_category, texture_type> _materials;
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_MODEL_INL
#include "./model.inl"
#endif
