#include "./gl_buffer.hpp"

namespace shogle {

static constexpr gl_bitfield INMUTABLE_FLAG = 1 << 31;
static constexpr gl_bitfield PROPERTIES_BITS = ~INMUTABLE_FLAG;

gl_sv_expect<gl_buffer> gl_make_buffer(gl_context& gl, gl_buffer::buffer_type type, size_t size,
                                       const void* data) {
  return gl_buffer::create(gl, type, gl_buffer::USAGE_DYNAMIC_STORAGE_BIT, data, size);
}

gl_sv_expect<gl_buffer> gl_buffer::create(gl_context& gl, gl_buffer::buffer_type type,
                                          gl_buffer::buffer_bits usage, const void* data,
                                          size_t size) {
  if (data) {
    NTF_ASSERT((usage & USAGE_DYNAMIC_STORAGE_BIT),
               "Attempted to create inmutable buffer with no data");
  }

  GLuint id;
  GL_ASSERT(glGenBuffers(1, &id));
  const GLenum gltype = static_cast<GLenum>(type);
  GL_ASSERT(glBindBuffer(gltype, id));

  const auto err = GL_RET_ERR(glBufferStorage(gltype, size, data, (GLbitfield)usage));
  if (err != GL_NO_ERROR) {
    GL_ASSERT(glBindBuffer(gltype, NULL_BINDING));
    GL_ASSERT(glDeleteBuffers(1, &id));
    return {ntf::unexpect, "Failed to create buffer", err};
  }
  return {ntf::in_place, id, type, usage | INMUTABLE_FLAG, size};
}

gl_sv_expect<gl_buffer> gl_buffer::create_mutable(gl_context& gl, gl_buffer::buffer_type type,
                                                  gl_buffer::buffer_mut_usage usage,
                                                  const void* data, size_t size) {
  GLuint id;
  GL_ASSERT(glGenBuffers(1, &id));
  const GLenum gltype = static_cast<GLenum>(type);
  GL_ASSERT(glBindBuffer(gltype, id));
  const auto err = GL_RET_ERR(glBufferData(id, size, data, usage));
  if (err != GL_NO_ERROR) {
    GL_ASSERT(glBindBuffer(gltype, NULL_BINDING));
    GL_ASSERT(glDeleteBuffers(1, &id));
    return {ntf::unexpect, "Failed to create mutable buffer", err};
  }

  return {ntf::in_place, id, type, usage, size};
}

void gl_buffer::destroy(gl_context& gl) {
  NTF_UNUSED(gl);
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  GL_ASSERT(glDeleteBuffers(1, &_id));
  _id = NULL_BINDING;
}

void gl_buffer::upload_data(gl_context& gl, const void* data, size_t size, size_t offset) {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  NTF_ASSERT(size + offset <= _size, "Out of bounds buffer writting");
  GL_ASSERT(glBindBuffer((GLenum)_type, _id));
  GL_ASSERT(glBufferSubData((GLenum)_type, offset, size, data));
}

gl_expect<void*> gl_buffer::map_range(gl_context& gl, size_t size, size_t offset,
                                      gl_buffer::mapping_bits access_flags) {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  GL_ASSERT(glBindBuffer((GLenum)_type, _id));
  void* ptr = GL_CALL(glMapBufferRange((GLenum)_type, offset, size, (GLbitfield)access_flags));
  const auto err = gl_get_error(gl);
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

gl_expect<void*> gl_buffer::map(gl_context& gl, gl_buffer::mapping_access access) {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  GL_ASSERT(glBindBuffer((GLenum)_type, _id));
  void* ptr = GL_CALL(glMapBuffer((GLenum)_type, access));
  const auto err = gl_get_error(gl);
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

void gl_buffer::unmap(gl_context& gl) {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  GL_ASSERT(glBindBuffer((GLenum)_type, _id));
  GL_ASSERT(glUnmapBuffer((GLenum)_type));
}

gl_handle gl_buffer::id() const {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  return _id;
}

gl_buffer::buffer_type gl_buffer::type() const {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  return _type;
}

u32 gl_buffer::properties() const {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  return _props & PROPERTIES_BITS;
}

size_t gl_buffer::size() const {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  return _size;
}

bool gl_buffer::is_mutable() const {
  NTF_ASSERT(_id != NULL_BINDING, "gl_buffer use after free");
  return !(_props & INMUTABLE_FLAG);
}

} // namespace shogle
