#include <shogle/render/model.hpp>

#include <shogle/render/shader.hpp>

namespace ntf::render {

model::model(loader_t loader) {
  for (auto& mesh : loader.meshes) {
    _meshes.emplace_back(std::move(mesh));
  }
}

void model::draw(shader& shader) const {
  shader.use();
  for (const auto& mesh : _meshes) {
    for (size_t i = 0; i < mesh.materials.size(); ++i) {
      const auto& material = mesh.materials[i];

      renderer::texture_bind(material.tex, i);
      shader.set_uniform(material.uniform_name.c_str(), i);
    }
    shader.set_uniform("material.shiny", 1.0f); // TODO: Don't hardcode this

    renderer::draw_mesh(mesh);
  }
}

} // namespace ntf::render
