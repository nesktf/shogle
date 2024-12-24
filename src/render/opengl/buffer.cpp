#include "./buffer.hpp"

namespace ntf {

void gl_buffer::load(r_buffer_type type, const void* data, size_t size) {
  NTF_ASSERT(!_id);

  const GLenum gltype = gl_buffer_type_cast(type);
  NTF_ASSERT(gltype);

  const GLbitfield glflags = GL_DYNAMIC_STORAGE_BIT;
  NTF_ASSERT(glflags);

  GLuint id;
  glGenBuffers(1, &id);
  glBindBuffer(gltype, id);
  glBufferStorage(gltype, size, nullptr, glflags);
  glCheckError();
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

} // namespace ntf
