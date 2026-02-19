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
    return {unexpect, GL_INVALID_VALUE};
  }

  GLuint vao;
  const auto err = GL_RET_ERR(glCreateVertexArrays(1, &vao));
  if (err) {
    return {unexpect, err};
  }

  attribute_array attributes;
  std::memcpy(attributes.data(), attribs, attrib_count * sizeof(attributes[0]));
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  SHOGLE_GL_LOG(VERBOSE, "VAO_CREATE ({}) [stride: {}B, type: {}]", vao, stride,
                stride ? "AOS" : "SOA");
  for (u32 i = 0; i < attrib_count; ++i) {
    auto attrib = attributes[i];
    SHOGLE_GL_LOG(VERBOSE, "- (loc: {}, off: {}, type: {})", attrib.location, attrib.offset,
                  ::shogle::meta::attribute_name(attrib.type));
  }
#endif
  return {in_place, create_t{}, vao, std::move(attributes), attrib_count, stride};
}

void gl_vertex_layout::destroy(gl_context& gl, gl_vertex_layout& layout) noexcept {
  if (SHOGLE_UNLIKELY(layout.invalidated())) {
    return;
  }
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  SHOGLE_GL_LOG(VERBOSE, "VAO_DESTROY ({}) [stride: {}B, type: {}]", layout._vao, layout._stride,
                layout._stride ? "AOS" : "SOA");
  for (const auto& attrib : layout.attributes()) {
    SHOGLE_GL_LOG(VERBOSE, "- (loc: {}, off: {}, type: {})", attrib.location, attrib.offset,
                  ::shogle::meta::attribute_name(attrib.type));
  }
#endif
  GL_CALL(glDeleteVertexArrays(1, &layout._vao));
  layout._vao = GL_NULL_HANDLE;
}

void gl_vertex_layout::destroy_n(gl_context& gl, gl_vertex_layout* layouts,
                                 size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!layouts)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(layouts[i].invalidated())) {
      continue;
    }
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
    SHOGLE_GL_LOG(VERBOSE, "VAO_DESTROY ({}) [stride: {}, type: {}]", layouts[i]._vao,
                  layouts[i]._stride, layouts[i]._stride ? "AOS" : "SOA");
    for (const auto& attrib : layouts[i].attributes()) {
      SHOGLE_GL_LOG(VERBOSE, "- (loc: {}, off: {}, type: {})", attrib.location, attrib.offset,
                    ::shogle::meta::attribute_name(attrib.type));
    }
#endif
    GL_CALL(glDeleteVertexArrays(1, &layouts[i]._vao));
    layouts[i]._vao = GL_NULL_HANDLE;
  }
}

void gl_vertex_layout::destroy_n(gl_context& gl, span<gl_vertex_layout> layouts) noexcept {
  destroy_n(gl, layouts.data(), layouts.size());
}

gldefs::GLhandle gl_vertex_layout::vao() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _vao;
}

span<const vertex_attribute> gl_vertex_layout::attributes() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return {_attributes.data(), _attribute_count};
}

size_t gl_vertex_layout::stride() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _stride;
}

auto gl_vertex_layout::type() const -> layout_type {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _stride ? TYPE_AOS_LAYOUT : TYPE_SOA_LAYOUT;
}

bool gl_vertex_layout::invalidated() const noexcept {
  return _vao == GL_NULL_HANDLE;
}

} // namespace shogle
