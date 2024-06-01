#include <shogle/render/gl/mesh.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

namespace ntf::shogle::gl {

mesh::mesh() { 
  glGenVertexArrays(1, &_vao); 
  log::verbose("[gl::mesh] Mesh created (id: {})", _vao);
}

mesh::mesh(mesh&& m) noexcept :
  _vao(m._vao), _vbo(m._vbo), _ebo(m._ebo),
  _draw_count(m._draw_count), _attrib_count(m._attrib_count) { 
  m._vao = 0;
}

mesh& mesh::operator=(mesh&& m) noexcept {
  auto id = _vao;

  glBindVertexArray(_vao);
  for (size_t i = 0; i < _attrib_count; ++i) {
    glDisableVertexAttribArray(i);
  }
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &_vao);

  if (_ebo) {
    glDeleteBuffers(1, &_ebo);
  }
  if (_vbo) {
    glDeleteBuffers(1, &_vbo);
  }

  _vao = m._vao;
  _vbo = m._vbo;
  _ebo = m._ebo;
  _draw_count = m._draw_count;
  _attrib_count = m._attrib_count;

  m._vao = 0;

  log::verbose("[gl::mesh] Mesh overwritten (id: {})", id);
  return *this;
}

mesh::~mesh() {
  if (!_vao) return;
  auto id = _vao;

  glBindVertexArray(_vao);
  for (size_t i = 0; i < _attrib_count; ++i) {
    glDisableVertexAttribArray(i);
  }
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &_vao);

  if (_ebo) {
    glDeleteBuffers(1, &_ebo);
  }
  if (_vbo) {
    glDeleteBuffers(1, &_vbo);
  }

  log::verbose("[gl::mesh] Mesh Destroyed (id: {})", id);
}

mesh& mesh::add_index_buffer(uint* indices, size_t ind_sz) {
  _draw_count = ind_sz; 

  glGenBuffers(1, &_ebo);
  glBindVertexArray(_vao);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_sz, indices, GL_STATIC_DRAW);

  glBindVertexArray(0);
  return *this;
}

void mesh::draw() const {
  if (!_vao) return;

  glBindVertexArray(_vao);
  if (this->has_indices()) {
    glDrawElements(GL_TRIANGLES, _draw_count, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, _draw_count);
  }
  glBindVertexArray(0);
}
//
// template<>
// void gl::mesh::_setup_attrib<gl::mesh::vertex2d>(void) {
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2d), (void*)0);
//
//   glEnableVertexAttribArray(1);
//   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2d), (void*)offsetof(vertex2d, tex_coord));
//
//   attrib_count = 2;
//   Log::verbose("[gl::mesh] Mesh created (id: {}, type: {})", vao, "vertex2d");
// }
//
// template<>
// void gl::mesh::_setup_attrib<gl::mesh::vertex3d>(void) {
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3dn), (void*)0);
//
//   glEnableVertexAttribArray(1);
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3dn), (void*)offsetof(vertex3d, normal));
//
//   glEnableVertexAttribArray(2);
//   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)offsetof(vertex3d, tex_coord));
//
//   attrib_count = 3;
//   Log::verbose("[gl::mesh] Mesh created (id: {}, type: {})", vao, "vertex3d");
// }
//
// template<>
// void gl::mesh::_setup_attrib<gl::mesh::vertex3dn>(void) {
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3dn), (void*)0);
//
//   glEnableVertexAttribArray(1);
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3dn), (void*)offsetof(vertex3dn, normal));
//
//   attrib_count = 2;
//   Log::verbose("[gl::mesh] Mesh created (id: {}, type: {})", vao, "vertex3dn");
// }
//
// template<>
// void gl::mesh::_setup_attrib<vec3>(void) {
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
//
//   attrib_count = 1;
//   Log::verbose("[gl::mesh] Mesh created (id: {}, type: {})", vao, "vec3");
// }

} // namespace ntf::shogle::gl
