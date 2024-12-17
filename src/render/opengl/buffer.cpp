#include "buffer.hpp"

namespace ntf {

void gl_buffer::load(r_buffer_type type, const void* data, size_t size) {
  NTF_ASSERT(!_id);

  const GLenum gltype = gl_buffer_type_cast(type);
  NTF_ASSERT(gltype);

  const GLbitfield glflags = GL_DYNAMIC_STORAGE_BIT;
  NTF_ASSERT(glflags);

  GLuint id;
  glGenBuffers(1, &id);
  glBindBuffer(gltype, _id);
  glBufferStorage(gltype, size, nullptr, glflags);
  if (data) {
    glBufferSubData(gltype, 0, size, data);
  }
  glBindBuffer(gltype, 0);

  _id = id;
  _type = type;
  _size = size;
  _alloc_flags = glflags;
}

void gl_buffer::unload() {
  NTF_ASSERT(_id);

  glDeleteBuffers(1, &_id);

  _id = 0;
  _type = r_buffer_type::none;
  _size = 0;
  _alloc_flags = 0;
}

void gl_buffer::data(const void* data, size_t size, size_t offset) {
  NTF_ASSERT(_id);
  NTF_ASSERT(_alloc_flags & GL_DYNAMIC_STORAGE_BIT);
  NTF_ASSERT(offset+size <= _size);

  const GLenum gltype = gl_buffer_type_cast(_type);
  NTF_ASSERT(gltype);

  glBindBuffer(gltype, _id);
  glBufferSubData(gltype, offset, size, data);
  glBindBuffer(gltype, 0);
}

// enum class storage_type {
//   none          = GL_NONE,
//   static_draw   = GL_STATIC_DRAW,
//   dynamic_draw  = GL_DYNAMIC_DRAW,
//   stream_draw   = GL_STREAM_DRAW,
// };
//
// enum class alloc_flags : uint32 {
//   none = 0,
//   dynamic_storage = GL_DYNAMIC_STORAGE_BIT,
//   map_read        = GL_MAP_READ_BIT,
//   map_write       = GL_MAP_WRITE_BIT,
//   map_persistent  = GL_MAP_PERSISTENT_BIT,
//   map_coherent    = GL_MAP_COHERENT_BIT,
//   client_storage  = GL_CLIENT_STORAGE_BIT,
// };
// NTF_DEFINE_ENUM_CLASS_FLAG_OPS(gl_buffer::alloc_flags);

GLenum gl_buffer_type_cast(r_buffer_type type) {
  switch(type) {
    case r_buffer_type::index:    return GL_ARRAY_BUFFER;
    case r_buffer_type::vertex:   return GL_ELEMENT_ARRAY_BUFFER;
    case r_buffer_type::uniform:  return GL_UNIFORM_BUFFER;

    case r_buffer_type::none:     return 0;
  };
  NTF_UNREACHABLE();
}

} // namespace ntf
