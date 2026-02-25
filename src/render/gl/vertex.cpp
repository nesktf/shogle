#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/vertex.hpp>

namespace shogle {

gl_layout_builder::gl_layout_builder() noexcept :
    _vert(), _ind(), _ind_format(), _vert_offset(), _ind_offset() {}

gl_layout_builder& gl_layout_builder::set_vertex_buffer(gl_buffer& buffer) {
  _vert = buffer;
  return *this;
}

gl_layout_builder&
gl_layout_builder::set_index_buffer(gl_buffer& buffer,
                                    gl_vertex_layout::index_buffer_format format) {
  _ind_format = format;
  _ind = buffer;
  return *this;
}

gl_layout_builder& gl_layout_builder::set_vertex_offset(size_t offset) {
  _vert_offset = offset;
  return *this;
}

gl_layout_builder& gl_layout_builder::set_index_offset(size_t offset) {
  _ind_offset = offset;
  return *this;
}

gl_vertex_layout::gl_vertex_layout(create_t, attribute_array attributes, u32 attribute_count,
                                   gldefs::GLhandle vao, optional<gl_buffer> vertex,
                                   optional<gl_buffer> index, index_buffer_format format,
                                   size_t vertex_offset, size_t index_offset) :
    _attributes(attributes), _attribute_count(attribute_count), _vao(vao), _vertex(vertex),
    _index(index), _format(format), _vertex_offset(vertex_offset), _index_offset(index_offset) {}

namespace {

gldefs::GLenum underlying_attribute_type(attribute_type attrib) {
  static constexpr auto attrs = std::to_array({
    GL_FLOAT,        // f32
    GL_FLOAT,        // vec2
    GL_FLOAT,        // vec3
    GL_FLOAT,        // vec4
    GL_FLOAT,        // mat3
    GL_FLOAT,        // mat4
    GL_DOUBLE,       // f64
    GL_DOUBLE,       // dvec2
    GL_DOUBLE,       // dvec3
    GL_DOUBLE,       // dvec4
    GL_INT,          // i32
    GL_INT,          // ivec2
    GL_INT,          // ivec3
    GL_INT,          // ivec4
    GL_UNSIGNED_INT, // u32
    GL_UNSIGNED_INT, // uvec2
    GL_UNSIGNED_INT, // uvec3
    GL_UNSIGNED_INT, // uvec4
  });
  const u32 idx = static_cast<u32>(attrib);
  return idx < attrs.size() ? attrs[idx] : 0;
};

u32 attribute_dimension(attribute_type attrib) {
  return ::shogle::meta::attribute_dim(attrib);
}

} // namespace

gl_expect<gl_vertex_layout>
gl_vertex_layout::_create(gl_context& gl, span<const vertex_attribute> attribs,
                          ptr_view<gl_buffer> vertex_buffer, ptr_view<gl_buffer> index_buffer,
                          index_buffer_format format, size_t vertex_offset, size_t index_offset) {
  if (attribs.empty()) {
    return {unexpect, GL_INVALID_VALUE};
  }
  SHOGLE_ASSERT(attribs.size() < MAX_ATTRIBUTE_BINDINGS, "Attribute count out of range");

  const auto bind_attrib_pointer = [&](shogle::attribute_type type, u32 location, size_t stride,
                                       size_t offset_) {
    void* offset = reinterpret_cast<void*>(offset_);
    const u32 dimension = attribute_dimension(type);
    const auto underlying = underlying_attribute_type(type);

    GL_ASSERT(glEnableVertexAttribArray(location));
    switch (underlying) {
      case GL_FLOAT: {
        GL_ASSERT(
          glVertexAttribPointer(location, dimension, underlying, GL_FALSE, stride, offset));
      } break;
      case GL_DOUBLE: {
        GL_ASSERT(glVertexAttribLPointer(location, dimension, underlying, stride, offset));
      } break;
      case GL_INT: {
        GL_ASSERT(glVertexAttribIPointer(location, dimension, underlying, stride, offset));
      } break;
      default:
        SHOGLE_UNREACHABLE();
    }
  };

  optional<gl_buffer> vertex, index;
  GLuint vao;
  const auto err = GL_RET_ERR(glCreateVertexArrays(1, &vao));
  if (err) {
    return {unexpect, err};
  }
  GL_ASSERT(glBindVertexArray(vao));
  if (vertex_buffer) {
    SHOGLE_ASSERT(vertex_offset < vertex_buffer->size(), "Invalid vertex offset");
    GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id()));
    vertex.emplace(*vertex_buffer);
  }
  for (const auto& attrib : attribs) {
    bind_attrib_pointer(attrib.type, attrib.location, attrib.stride, attrib.offset);
  }
  if (index_buffer) {
    SHOGLE_ASSERT(index_offset < index_buffer->size(), "Invalid index offset");
    GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer->id()));
    index.emplace(*index_buffer);
  }
  GL_ASSERT(glBindVertexArray(GL_DEFAULT_BINDING));

  attribute_array attributes;
  std::memcpy(attributes.data(), attribs.data(), attribs.size_bytes());
  u32 attrib_count = (u32)attribs.size();

  SHOGLE_GL_LOG(VERBOSE, "VAO_CREATE ({}) [vert: {}, indx: {}]", vao,
                vertex ? (i32)vertex->id() : -1, index ? (i32)index->id() : -1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  for (u32 i = 0; i < attrib_count; ++i) {
    auto attrib = attributes[i];
    SHOGLE_GL_LOG(VERBOSE, "- [type: {}, loc: {}, stride: {}B, off: {}B]",
                  ::shogle::meta::attribute_name(attrib.type), attrib.location, attrib.stride,
                  attrib.offset);
  }
#endif
  return {in_place, create_t{}, attributes, attrib_count,  vao,
          vertex,   index,      format,     vertex_offset, index_offset};
}

void gl_vertex_layout::destroy(gl_context& gl, gl_vertex_layout& layout) noexcept {
  if (SHOGLE_UNLIKELY(layout._vao == GL_NULL_HANDLE)) {
    return;
  }
  SHOGLE_GL_LOG(VERBOSE, "VAO_DESTROY ({}) [vert: {}, indx: {}]", layout._vao,
                layout._vertex ? (i32)layout._vertex->id() : -1,
                layout._index ? (i32)layout._index->id() : -1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  for (const auto& attrib : layout.attributes()) {
    SHOGLE_GL_LOG(VERBOSE, "- [type: {}, loc: {}, stride: {}B, off: {}B]",
                  ::shogle::meta::attribute_name(attrib.type), attrib.location, attrib.stride,
                  attrib.offset);
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
    if (SHOGLE_UNLIKELY(layouts[i]._vao == GL_NULL_HANDLE)) {
      continue;
    }
    SHOGLE_GL_LOG(VERBOSE, "VAO_DESTROY ({}) [vert: {}, indx: {}]", layouts[i]._vao,
                  layouts[i]._vertex ? (i32)layouts[i]._vertex->id() : -1,
                  layouts[i]._index ? (i32)layouts[i]._index->id() : -1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
    for (const auto& attrib : layouts[i].attributes()) {
      SHOGLE_GL_LOG(VERBOSE, "- [type: {}, loc: {}, stride: {}B, off: {}B]",
                    ::shogle::meta::attribute_name(attrib.type), attrib.location, attrib.stride,
                    attrib.offset);
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

const gl_buffer* gl_vertex_layout::vertex_buffer() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _vertex ? &*_vertex : nullptr;
}

size_t gl_vertex_layout::vertex_offset() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _vertex_offset;
}

const gl_buffer* gl_vertex_layout::index_buffer() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _index ? &*_index : nullptr;
}

size_t gl_vertex_layout::index_offset() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _index_offset;
}

gl_vertex_layout::index_buffer_format gl_vertex_layout::index_format() const {
  SHOGLE_ASSERT(_vao != GL_NULL_HANDLE, "gl_vertex_layout use after free");
  return _format;
}

} // namespace shogle
