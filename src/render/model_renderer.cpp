#include "render/model_renderer.hpp"

#include "core/engine.hpp"
#include "core/log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

void ModelRenderer::draw(glm::mat4 model_m) {
  const auto& eng = Shogle::instance();

  shader->use();
  shader->unif_mat4("proj", eng.proj3d);
  shader->unif_mat4("view", eng.view);
  shader->unif_vec3("view_pos", eng.view_pos);
  shader->unif_mat4("model", model_m);

  for (const auto& mesh : model->meshes) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < mesh.tex.size(); ++i) {
      const auto& tex = mesh.tex[i];
      tex.bind_material(*shader, tex.type() == aiTextureType_DIFFUSE ? diff_n++ : spec_n++, i);
    }

    glBindVertexArray(mesh.id());
    glDrawElements(GL_TRIANGLES, mesh.ind(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

} // namespace ntf
