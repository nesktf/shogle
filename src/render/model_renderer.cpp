#include "render/model.hpp"

#include "engine.hpp"
#include "log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle::render {

void Model::draw(void) {
  const auto& eng = Engine::instance();
  const auto& shader = this->shader_ref.get();
  const auto& model = this->model_ref.get();

  shader.use();
  shader.unif_mat4("proj", eng.proj3d);
  shader.unif_mat4("view", eng.view);
  shader.unif_vec3("view_pos", eng.view_pos);
  shader.unif_mat4("model", this->model_m);

  for (const auto& mesh : model.meshes) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < mesh.tex.size(); ++i) {
      const auto& tex = mesh.tex[i];
      tex.bind_material(shader, tex.type() == aiTextureType_DIFFUSE ? diff_n++ : spec_n++, i);
    }

    glBindVertexArray(mesh.id());
    glDrawElements(GL_TRIANGLES, mesh.ind(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

} // namespace ntf::shogle
