#include <shogle/render/model.hpp>

namespace ntf::render {

model::model(std::string path) :
  model(loader_t{std::move(path)}) {}

model::model(loader_t loader) {
  for (auto& mesh_loader : loader.meshes) {
    _meshes.emplace_back(gl::mesh{std::move(mesh_loader)});
  }
}

model& model::operator=(model&& m) noexcept {
  for (auto& _mesh : _meshes) {
    gl::destroy_mesh(_mesh);
  }

  _meshes = std::move(m._meshes);

  return *this;
}

model::~model() {
  for (auto& _mesh : _meshes) {
    gl::destroy_mesh(_mesh);
  }
}

void draw_model(model& model, shader& shader) {
  shader.use();
  for (const auto& mesh : model._meshes) {
    for (size_t i = 0; i < mesh.materials.size(); ++i) {
      const auto& material = mesh.materials[i];

      gl::texture_bind(material.tex, i);
      shader.set_uniform(material.uniform_name.c_str(), i);
    }
    shader.set_uniform("material.shiny", 1.0f); // TODO: Don't hardcode this

    gl::draw_mesh(mesh);
  }
}


cubemap::cubemap(std::string path) :
  cubemap(loader_t{std::move(path)}) {}

cubemap::cubemap(loader_t loader) :
  _tex(std::move(loader)) {}

cubemap& cubemap::operator=(cubemap&& c) noexcept {
  gl::destroy_texture(_tex);

  _tex = std::move(c._tex);

  c._tex.id = 0;

  return *this;
}

cubemap::~cubemap() {
  gl::destroy_texture(_tex);
}

void draw_cubemap(cubemap& cube, shader& shader) {
  shader.use();
  shader.set_uniform("skybox", 0);

  gl::texture_bind(cube._tex);
  gl::draw_cubemap();
}

} // namespace ntf::render
