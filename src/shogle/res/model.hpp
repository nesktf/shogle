#pragma once

#include <shogle/core/types.hpp>

#include <shogle/res/loader_data.hpp>

#include <shogle/render/mesh.hpp>

#include <vector>

namespace ntf::shogle {

class model_mesh {
public:
  texture2d& find_material(material_type type);
  class mesh& mesh() { return _mesh; }

private:
  class mesh _mesh {};
  std::string _name;
  std::vector<std::pair<material_type, texture2d>> _materials;
  friend class model;
};

class model {
public:
  model(model_loader data);

  model(std::string_view path) :
    model(model_loader{path}) {}

public:
  model_mesh& find_mesh(std::string_view name);

  model_mesh& operator[](size_t index) { return _meshes[index]; }
  model_mesh& at(size_t index) { return _meshes.at(index); }

  std::vector<model_mesh>::iterator begin() { return _meshes.begin(); }
  std::vector<model_mesh>::iterator end() { return _meshes.end(); }

  size_t size() const { return _meshes.size(); }

private:
  std::vector<model_mesh> _meshes;
};

} // namespace ntf::shogle
