#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/vertex.hpp>

namespace shogle {

gl_vertex_layout::gl_vertex_layout(create_t, gldefs::GLhandle vao, attribute_array attributes,
                                   u32 attribute_count, size_t stride) :
    _attributes(std::move(attributes)), _stride(stride), _vao(vao),
    _attribute_count(attribute_count) {}

gl_expect<gl_vertex_layout> gl_vertex_layout::create(gl_context& gl, size_t stride,
                                                     const ::shogle::vertex_attribute* attribs,
                                                     u32 attrib_count) {
  if (!attribs || !attrib_count) {
    return {ntf::unexpect, GL_INVALID_VALUE};
  }

  GLuint vao;
  const auto err = GL_RET_ERR(glCreateVertexArrays(1, &vao));
  if (err) {
    return {ntf::unexpect, err};
  }

  attribute_array attributes;
  std::memcpy(attributes.data(), attribs, attrib_count * sizeof(attributes[0]));
  return {ntf::in_place, create_t{}, vao, std::move(attributes), attrib_count, stride};
}

void gl_vertex_layout::destroy(gl_context& gl, gl_vertex_layout& layout) {
  if (layout.invalidated()) {
    return;
  }
  GL_CALL(glDeleteVertexArrays(1, &layout._vao));
  layout._vao = GL_NULL_HANDLE;
}

gldefs::GLhandle gl_vertex_layout::vao() const {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _vao;
}

span<const vertex_attribute> gl_vertex_layout::attributes() const {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return {_attributes.data(), _attribute_count};
}

size_t gl_vertex_layout::stride() const {
  NTF_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _stride;
}

bool gl_vertex_layout::invalidated() const noexcept {
  return _vao == GL_NULL_HANDLE;
}

} // namespace shogle
