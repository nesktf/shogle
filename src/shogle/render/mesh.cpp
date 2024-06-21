#include <shogle/render/mesh.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

namespace ntf::shogle {

mesh::mesh() { 
  glGenVertexArrays(1, &_vao); 
  log::verbose("[shogle::mesh] Mesh created (id: {})", _vao);
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

  log::verbose("[shogle::mesh] Mesh Destroyed (id: {})", id);
}

mesh::mesh(mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _draw_count(std::move(m._draw_count)), _attrib_count(std::move(m._attrib_count)) { 
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

  log::verbose("[shogle::mesh] Mesh overwritten (id: {})", id);
  return *this;
}

} // namespace ntf::shogle::gl
