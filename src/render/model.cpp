#include "render/model.hpp"
#include "log.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

// TODO: Update proj matrix on screen resize
const glm::mat4 proj = glm::perspective(
  glm::radians(45.0f),
  800.0f/600.0f,
  0.1f,
  100.0f
);

// TODO: Set a proper 3d camera
const glm::vec3 view_pos = {0.0f, 0.0f, 0.0f};
const glm::vec3 view_dir = {0.0f, 0.0f, -1.0f};
const glm::vec3 view_up =  {0.0f, 1.0f, 0.0f};
const glm::mat4 view = glm::lookAt(
  view_pos,
  view_pos + view_dir,
  view_up
);
// this->dir_vec = glm::normalize(glm::vec3{
//   glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
//   glm::sin(glm::radians(pitch)),
//   glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
// });

void Model::draw(void) {
  shader->unif_mat4("proj", proj);
  shader->unif_mat4("view", view);
  shader->unif_vec3("view_pos", view_pos);
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

} // namespace ntf::shogle
