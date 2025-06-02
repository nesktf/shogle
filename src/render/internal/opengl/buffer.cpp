#include "./state.hpp"

namespace ntf {

GLenum gl_state::buffer_type_cast(r_buffer_type type) noexcept {
  switch(type) {
    case r_buffer_type::vertex:         return GL_ARRAY_BUFFER;
    case r_buffer_type::index:          return GL_ELEMENT_ARRAY_BUFFER;
    case r_buffer_type::uniform:        return GL_UNIFORM_BUFFER;
    case r_buffer_type::texel:          return GL_TEXTURE_BUFFER;
    case r_buffer_type::shader_storage: return GL_SHADER_STORAGE_BUFFER;
  };

  NTF_UNREACHABLE();
}

GLenum& gl_state::_buffer_pos(GLenum type) {
  switch (type) {
    case GL_ARRAY_BUFFER:           return _bound_buffers[BUFFER_TYPE_VERTEX];
    case GL_ELEMENT_ARRAY_BUFFER:   return _bound_buffers[BUFFER_TYPE_INDEX];
    case GL_TEXTURE_BUFFER:         return _bound_buffers[BUFFER_TYPE_TEXEL];
    case GL_UNIFORM_BUFFER:         return _bound_buffers[BUFFER_TYPE_UNIFORM];
    case GL_SHADER_STORAGE_BUFFER:  return _bound_buffers[BUFFER_TYPE_SHADER];
  }

  NTF_UNREACHABLE();
}

auto gl_state::create_buffer(r_buffer_type type, r_buffer_flag flags, size_t size,
                             weak_cptr<r_buffer_data> data) -> buffer_t {

  const bool is_dynamic = +(flags & r_buffer_flag::dynamic_storage);
  const GLbitfield access_flags = 
    (+(flags & r_buffer_flag::read_mappable) || +(flags & r_buffer_flag::write_mappable) ?
      GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT : 0) |
    (+(flags & r_buffer_flag::read_mappable) ? GL_MAP_READ_BIT : 0) |
    (+(flags & r_buffer_flag::write_mappable) ? GL_MAP_WRITE_BIT : 0);

  const GLbitfield glflags = access_flags |
    (is_dynamic ? GL_DYNAMIC_STORAGE_BIT : 0);

  if (!is_dynamic) {
    NTF_ASSERT(data);
    NTF_ASSERT(data->data);
    NTF_ASSERT(data->offset+data->size <= size);
  }
  GLuint id;
  GL_CALL(glGenBuffers, 1, &id);
  const GLenum gltype = buffer_type_cast(type);

  _buffer_pos(gltype) = id;

  GLuint last = _buffer_pos(gltype);
  GL_CALL(glBindBuffer, gltype, id);
  const void* data_ptr = !data ? nullptr :
    reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data->data) + data->offset);
  if (auto err = GL_CHECK(glBufferStorage, gltype, size, data_ptr, glflags); err != GL_NO_ERROR) {
    GL_CALL(glBindBuffer, gltype, last);
    GL_CALL(glDeleteBuffers, 1, &id);
    throw error<>::format({"Failed to create buffer with size {}"}, size);
  }
  _buffer_pos(gltype) = id;

  buffer_t buff;
  buff.id = id;
  buff.size = size;
  buff.type = gltype;
  buff.flags = glflags;
  buff.mapping = access_flags;
  buff.mapping_ptr = nullptr;
  return buff;
}

void gl_state::destroy_buffer(const buffer_t& buffer) noexcept {
  NTF_ASSERT(buffer.id);
  GLuint id = buffer.id;
  GLenum& pos = _buffer_pos(buffer.type);
  if (pos == id) {
    GL_CALL(glBindBuffer, buffer.type, NULL_BINDING);
    pos = NULL_BINDING;
  }
  GL_CALL(glDeleteBuffers, 1, &id);
}

bool gl_state::bind_buffer(GLuint id, GLenum type) noexcept {
  GLenum& pos = _buffer_pos(type);
  if (pos == id) {
    return false;
  }
  GL_CALL(glBindBuffer, type, id);
  pos = id;
  return true;
}

void gl_state::update_buffer(const buffer_t& buffer, const void* data,
                             size_t size, size_t off) noexcept {
  NTF_ASSERT(buffer.flags & GL_DYNAMIC_STORAGE_BIT);
  NTF_ASSERT(size+off <= buffer.size);

  bind_buffer(buffer.id, buffer.type);
  GL_CALL(glBufferSubData, buffer.type, off, size, data);
}

void* gl_state::map_buffer(buffer_t& buffer, size_t offset, size_t len) {
  NTF_ASSERT(offset+len <= buffer.size);
  NTF_ASSERT(buffer.mapping != 0);

  if (buffer.mapping_ptr) {
    unmap_buffer(buffer, buffer.mapping_ptr);
  }

  bind_buffer(buffer.id, buffer.type);
  void* ptr = GL_CALL_RET(glMapBufferRange, buffer.type, offset, len, buffer.mapping);
  buffer.mapping_ptr = ptr;
  return ptr;
}

void gl_state::unmap_buffer(buffer_t& buffer, void* ptr) {
  if (buffer.mapping_ptr != ptr || !ptr) {
    return;
  }
  bind_buffer(buffer.id, buffer.type);
  GL_CALL(glUnmapBuffer, buffer.type);
}

} // namespace ntf
