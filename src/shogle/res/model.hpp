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
  model_data(std::string_view path_, tex_filter mat_filter_, tex_wrap mat_wrap);

public:
  std::vector<mesh_data> meshes;
};


class textured_mesh {
public:
  textured_mesh(mesh mesh, pair_vector<material_type, texture2d> materials);

public:
  const mesh& get_mesh() const { return _mesh; }

  const texture2d& operator[](material_type material) const;
  const texture2d& operator[](size_t index) const { return _materials[index].second; }

  pair_vector<material_type, texture2d>::const_iterator begin() const { return _materials.begin(); }
  pair_vector<material_type, texture2d>::const_iterator end() const { return _materials.end(); }

  size_t size() const { return _materials.size(); }

private:
  shogle::mesh _mesh;
  pair_vector<material_type, texture2d> _materials;
};

class model {
public:
  model(std::vector<mesh_data> meshes);

public:
  const textured_mesh& operator[](std::string_view name) const;
  const textured_mesh& operator[](size_t index) const { return _meshes[index].second; }

  pair_vector<std::string, textured_mesh>::const_iterator begin() const { return _meshes.begin(); }
  pair_vector<std::string, textured_mesh>::const_iterator end() const { return _meshes.end(); }

  size_t size() const { return _meshes.size(); }

private:
  pair_vector<std::string, textured_mesh> _meshes;
};

model load_model(std::string_view path, tex_filter mat_filter, tex_wrap mat_wrap);
model load_model(model_data data);

} // namespace ntf::shogle
