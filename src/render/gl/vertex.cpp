#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/vertex.hpp>

namespace shogle {

gl_sv_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl,
                                                        span<const attribute_binding> binds) {
  if (binds.empty()) {
    return {ntf::unexpect, "No bindings provided", GL_INVALID_VALUE};
  }

  attribute_array attributes;
  std::memcpy(attributes.data(), binds.data(), binds.size() * sizeof(attributes[0]));
  const u32 attribute_count = static_cast<u32>(binds.size());

  GLuint vao;
  const auto err = GL_RET_ERR(glCreateVertexArrays(1, &vao));
  if (err) {
    return {ntf::unexpect, "Failed to create vertex array", err};
  }

  return {ntf::in_place, create_t{}, gl, vao, std::move(attributes), attribute_count};
}

gl_vertex_layout::gl_vertex_layout(create_t, gl_context& gl, gldefs::GLhandle vao,
                                   attribute_array attributes, u32 attribute_count) :
    _attributes(std::move(attributes)), _gl(gl), _vao(vao), _attribute_count(attribute_count) {}

gl_vertex_layout::gl_vertex_layout(gl_context& gl, span<const attribute_binding> binds) :
    gl_vertex_layout(::shogle::gl_vertex_layout::create(gl, binds).value()) {}

void gl_vertex_layout::destroy() {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  [[maybe_unused]] auto& gl = _gl.get();
  GL_CALL(glDeleteVertexArrays(1, &_vao));
  _vao = GL_NULL_HANDLE;
}

void gl_vertex_layout::rebind_context(gl_context& gl) {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  this->_gl = gl;
}

gldefs::GLhandle gl_vertex_layout::id() const {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _vao;
}

auto gl_vertex_layout::attributes() const -> span<const attribute_binding> {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return {_attributes.data(), _attribute_count};
}

gl_context& gl_vertex_layout::context() const {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _gl.get();
}

} // namespace shogle
