#include <shogle/res/model.hpp>

#include <shogle/core/error.hpp>

namespace ntf::shogle {

model::model(model_loader data) :
  _meshes(data.meshes.size()) {
  using att_coords    = shader_attribute<0, vec3>;
  using att_normals   = shader_attribute<1, vec3>;
  using att_texcoords = shader_attribute<2, vec2>;

  for (size_t i = 0; i < data.meshes.size(); ++i) {
    auto& mesh_data = data.meshes[i];
    auto& mesh = _meshes[i];

    mesh._name = std::move(mesh_data.name);
    for (auto& material : mesh_data.materials) {
      mesh._materials.emplace_back(std::make_pair(
        std::move(material.second),
        texture2d{
          vec2sz{material.first.width, material.first.height},
          material.first.format,
          tex_filter::linear,
          tex_wrap::repeat,
          std::move(material.first.pixels)
        }
      ));
    }
    mesh._mesh
      .add_vertex_buffer(&mesh_data.vertices[0], mesh_data.vertices.size()*sizeof(vertex3dnt),
        att_coords{}, att_normals{}, att_texcoords{})
      .add_index_buffer(&mesh_data.indices[0], mesh_data.indices.size()*sizeof(uint));
  }
}

model_mesh& model::find_mesh(std::string_view name) {
  auto it = std::find_if(_meshes.begin(), _meshes.end(), [name](const auto& mesh) {
    return mesh._name == name;
  });
  if (it != _meshes.end()) {
    return *it;
  }
  throw ntf::error{"[shogle::model] Mesh not found: {}", name};
}

texture2d& model_mesh::find_material(material_type type) {
  auto it = std::find_if(_materials.begin(), _materials.end(), [type](const auto& tex_pair) {
    return tex_pair.first == type;
  });
  if (it != _materials.end()) {
    return it->second;
  }
  throw ntf::error{"[shogle::model_mesh] Material type not found"};
}

} // namespace ntf::shogle
