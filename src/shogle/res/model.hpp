#pragma once

#include <shogle/render/mesh.hpp>

#include <shogle/res/texture.hpp>

namespace ntf::shogle {

enum class material_type {
  diffuse = 0,
  specular
};

struct vertex3d {
  vec3 coord;
  vec3 normal;
  vec2 tex_coord;
};

struct mesh_data {
  std::string name;
  std::vector<vertex3d> vertices;
  std::vector<uint> indices;
  pair_vector<material_type, texture2d_data> materials;
};

struct model_data {
public:
  model_data(std::string_view path_);

public:
  std::vector<mesh_data> meshes;
};


class textured_mesh {
public:
  textured_mesh(mesh mesh, std::unordered_map<material_type, texture2d> materials);

public:
  const mesh& get_mesh() const { return _mesh; }
  const texture2d& operator[](material_type material) const { return _materials.at(material); }

  auto cbegin() const { return _materials.cbegin(); }
  auto cend() const { return _materials.cend(); }
  auto begin() { return _materials.begin(); }
  auto end() { return _materials.end(); }

  size_t size() const { return _materials.size(); }

private:
  shogle::mesh _mesh;
  std::unordered_map<material_type, texture2d> _materials;
};

class model {
public:
  model() = default;
  model(std::vector<mesh_data> meshes, tex_filter mat_filter, tex_wrap mat_wrap);

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

inline model load_model(std::string_view path, tex_filter mat_filter, tex_wrap mat_wrap) {
  auto data = model_data{path};
  return model{std::move(data.meshes), mat_filter, mat_wrap};
}

} // namespace ntf::shogle
