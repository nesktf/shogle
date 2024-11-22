#include "./mesh.hpp"

namespace ntf {

gl_mesh& gl_mesh::indices(const uint* ind, std::size_t sz, mesh_buffer buff) & {
  _set_indices(ind, sz, buff);
  return *this;
}

gl_mesh&& gl_mesh::indices(const uint* ind, std::size_t sz, mesh_buffer buff) && {
  _set_indices(ind, sz, buff);
  return std::move(*this);
}

gl_mesh& gl_mesh::indices(const uint* ind, std::size_t sz, std::size_t offset) & {
  _set_indices(ind, sz, offset);
  return *this;
}

void gl_mesh::unload() {
  if (!_vao) {
    return;
  }
  SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh destroyed (id: {})", _vao);
  glDeleteVertexArrays(1, &_vao);
  if (_vbo) {
    glDeleteBuffers(1, &_vbo);
  }
  if (_ebo) {
    glDeleteBuffers(1, &_ebo);
  }
  _reset();
}


void gl_mesh::_set_indices(const uint* indices, std::size_t sz, mesh_buffer buff) {
  NTF_ASSERT(!_ebo);
  if (!_vao) {
    glGenVertexArrays(1, &_vao);
  }

  glGenBuffers(1, &_ebo);
  glBindVertexArray(_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, indices, enumtogl(buff));
  glBindVertexArray(0);
  _ebo_sz = sz;

  if (valid()) {
    SHOGLE_LOG(verbose, "[ntf::gl_mesh] Mesh created (id: {})", _vao);
  }
}

void gl_mesh::_set_indices(const uint* indices, std::size_t sz, std::size_t offset) {
  NTF_ASSERT(_ebo);
  NTF_ASSERT(offset+sz <= _ebo_sz);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, indices);
  glBindVertexArray(0);
}

void gl_mesh::_reset() {
  _vbo_sz = 0;
  _ebo_sz = 0;
  _attrib_count = 0;
  _vao = 0;
  _vbo = 0;
  _ebo = 0;
}


gl_mesh::~gl_mesh() noexcept { unload(); }

gl_mesh::gl_mesh(gl_mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _vbo_sz(std::move(m._vbo_sz)), _ebo_sz(std::move(m._vbo_sz)),
  _attrib_count(std::move(m._attrib_count)) { m._reset(); }

auto gl_mesh::operator=(gl_mesh&& m) noexcept -> gl_mesh& {
  unload();

  _vao = std::move(m._vao);
  _vbo = std::move(m._vbo);
  _ebo = std::move(m._ebo);
  _vbo_sz = std::move(m._vbo_sz);
  _ebo_sz = std::move(m._vbo_sz);
  _attrib_count = std::move(m._attrib_count);

  m._reset();

  return *this;
}

} // namespace ntf
