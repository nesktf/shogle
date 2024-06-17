#pragma once

#include <shogle/core/types.hpp>

#include <shogle/res/texture.hpp>

#include <shogle/render/gl/mesh.hpp>

#include <vector>

namespace ntf::shogle {

struct vertex3d {
  vec3 coord;
  vec3 normal;
  vec2 tex_coord;
};

enum class material {
  diffuse = 0,
  specular
};

struct model_data {
public:
  struct mesh_data {
    std::string name;
    std::vector<vertex3d> vertices;
    std::vector<uint> indices;
    std::vector<std::pair<texture2d::data_t, material>> materials;
  };

public:
  model_data(std::string _path);

public:
  std::string path;
  std::vector<mesh_data> meshes;
};


class model {
public:
  using data_t = model_data;

  struct mesh {
    std::string name;
    gl::mesh mesh;
    std::vector<std::pair<texture2d, material>> materials;
    texture2d& find_material(material type);
  };

public:
  model(std::string path);
  model(data_t data);

public:
  mesh& find_mesh(std::string name);

  mesh& operator[](size_t index) { return _meshes[index]; }

  std::vector<mesh>::iterator begin() { return _meshes.begin(); }
  std::vector<mesh>::iterator end() { return _meshes.end(); }

  size_t size() const { return _meshes.size(); }
  std::string path() const { return _path; }

private:
  std::string _path;
  std::vector<mesh> _meshes;
};

} // namespace ntf::shogle
