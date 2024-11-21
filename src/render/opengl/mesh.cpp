#include "./mesh.hpp"

namespace ntf {

void gl_mesh::unload() {
  if (_vao && _vbo) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::mesh] Unloaded (id: {})", _vao);
    glBindVertexArray(_vao);
    for (std::size_t i = 0; i < _attrib_count; ++i) {
      glDisableVertexAttribArray(i); // ???
    }
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    if (_ebo) {
      glDeleteBuffers(1, &_ebo);
    }
    _vao = 0;
    _vbo = 0;
    _ebo = 0;
  }
}

bool gl_mesh::draw() const {
  if (!valid()) {
    return false;
  }

  glBindVertexArray(_vao);
  const auto prim = enumtogl(primitive());
  if (has_indices()) {
    glDrawElements(prim, draw_count(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(prim, 0, draw_count());
  }
  glBindVertexArray(0);

  return true;
}

gl_mesh::~gl_mesh() noexcept { unload(); }

gl_mesh::gl_mesh(gl_mesh&& m) noexcept :
  _vao(std::move(m._vao)), _vbo(std::move(m._vbo)), _ebo(std::move(m._ebo)),
  _draw_count(std::move(m._draw_count)), _attrib_count(std::move(m._attrib_count)),
  _primitive(std::move(m._primitive)) { m._vao = 0; m._vbo = 0; m._ebo = 0; }

auto gl_mesh::operator=(gl_mesh&& m) noexcept -> gl_mesh& {
  unload();

  _vao = std::move(m._vao);
  _vbo = std::move(m._vbo);
  _ebo = std::move(m._ebo);
  _draw_count = std::move(m._draw_count);
  _attrib_count = std::move(m._attrib_count);
  _primitive = std::move(m._primitive);

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;

  return *this;
}

} // namespace ntf
