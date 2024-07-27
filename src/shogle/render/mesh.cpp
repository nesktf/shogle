#include <shogle/render/mesh.hpp>

#include <shogle/core/error.hpp>

namespace ntf::shogle {

constexpr GLint __enumtogl(mesh_primitive primitive) {
  switch (primitive) {
    case mesh_primitive::triangles:
      return GL_TRIANGLES;
    case mesh_primitive::triangle_strip:
      return GL_TRIANGLE_STRIP;
    case mesh_primitive::triangle_fan:
      return GL_TRIANGLE_FAN;
    case mesh_primitive::lines:
      return GL_LINES;
    case mesh_primitive::line_strip:
      return GL_LINE_STRIP;
    case mesh_primitive::line_loop:
      return GL_LINE_LOOP;
    case mesh_primitive::points:
      return GL_POINTS;
  }
  return 0; // shutup gcc
}

mesh::mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, GLuint ebo, size_t draw_count, size_t att_count) :
  _vao(vao), _vbo(vbo), _ebo(ebo),
  _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

mesh::mesh(mesh_primitive primitive, GLuint vao, GLuint vbo, size_t draw_count, size_t att_count) :
  _vao(vao) , _vbo(vbo),
  _draw_count(draw_count), _attrib_count(att_count), _primitive(primitive) {}

void mesh::unload_mesh() {
  log::verbose("[shogle::mesh] Mesh Destroyed (id: {})", _vao);
  glBindVertexArray(_vao);
  for (size_t i = 0; i < _attrib_count; ++i) {
    glDisableVertexAttribArray(i);
  }
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &_vao);
  glDeleteBuffers(1, &_vbo);
  if (_ebo) {
    glDeleteBuffers(1, &_ebo);
  }
}

mesh::~mesh() {
  if (_vao) {
    unload_mesh();
  }
}

mesh::mesh(mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _draw_count(std::move(m._draw_count)), _attrib_count(std::move(m._attrib_count)) { 
  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;
}

mesh& mesh::operator=(mesh&& m) noexcept {
  if (_vao) {
    unload_mesh();
  }

  _vao = std::move(m._vao);
  _vbo = std::move(m._vbo);
  _ebo = std::move(m._ebo);
  _draw_count = std::move(m._draw_count);
  _attrib_count = std::move(m._attrib_count);

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;

  return *this;
}


void render_draw_mesh(const mesh& mesh) {
  assert(mesh.valid() && "Mesh has no data");

  glBindVertexArray(mesh._vao);
  const auto prim = __enumtogl(mesh.primitive());
  if (mesh.has_indices()) {
    glDrawElements(prim, mesh.draw_count(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(prim, 0, mesh.draw_count());
  }
  glBindVertexArray(0);
}

} // namespace ntf::shogle
