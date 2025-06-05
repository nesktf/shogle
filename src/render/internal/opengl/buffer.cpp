#include "./context.hpp"

namespace ntf::render {

GLenum gl_state::buffer_type_cast(buffer_type type) noexcept {
  switch(type) {
    case buffer_type::vertex:         return GL_ARRAY_BUFFER;
    case buffer_type::index:          return GL_ELEMENT_ARRAY_BUFFER;
    case buffer_type::uniform:        return GL_UNIFORM_BUFFER;
    case buffer_type::texel:          return GL_TEXTURE_BUFFER;
    case buffer_type::shader_storage: return GL_SHADER_STORAGE_BUFFER;
  };

  NTF_UNREACHABLE();
}

GLenum& gl_state::_buffer_pos(GLenum type) {
  switch (type) {
    case GL_ARRAY_BUFFER:           return _bound_buffers[GLBUFFER_TYPE_VERTEX];
    case GL_ELEMENT_ARRAY_BUFFER:   return _bound_buffers[GLBUFFER_TYPE_INDEX];
    case GL_TEXTURE_BUFFER:         return _bound_buffers[GLBUFFER_TYPE_TEXEL];
    case GL_UNIFORM_BUFFER:         return _bound_buffers[GLBUFFER_TYPE_UNIFORM];
    case GL_SHADER_STORAGE_BUFFER:  return _bound_buffers[GLBUFFER_TYPE_SHADER];
  }

  NTF_UNREACHABLE();
}

ctx_buff_status gl_state::create_buffer(glbuffer_t& buffer, buffer_type type, buffer_flag flags,
                                        size_t size, weak_cptr<buffer_data> data)
{
  const bool is_dynamic = +(flags & buffer_flag::dynamic_storage);
  const GLbitfield access_flags = 
    (+(flags & buffer_flag::read_mappable) || +(flags & buffer_flag::write_mappable) ?
      GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT : 0) |
    (+(flags & buffer_flag::read_mappable) ? GL_MAP_READ_BIT : 0) |
    (+(flags & buffer_flag::write_mappable) ? GL_MAP_WRITE_BIT : 0);

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

  GLuint last = _buffer_pos(gltype);
  GL_CALL(glBindBuffer, gltype, id);
  const void* data_ptr = !data ? nullptr :
    reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data->data) + data->offset);
  if (auto err = GL_CHECK(glBufferStorage, gltype, size, data_ptr, glflags); err != GL_NO_ERROR) {
    GL_CALL(glBindBuffer, gltype, last);
    GL_CALL(glDeleteBuffers, 1, &id);
    _buffer_pos(gltype) = last;
    return CTX_BUFF_STATUS_ALLOC_FAILED;
  }
  _buffer_pos(gltype) = id;

  buffer.id = id;
  buffer.size = size;
  buffer.type = gltype;
  buffer.flags = glflags;
  buffer.mapping = access_flags;

  return CTX_BUFF_STATUS_OK;
}

void gl_state::destroy_buffer(glbuffer_t& buffer) {
  NTF_ASSERT(buffer.id);
  GLuint id = buffer.id;
  GLenum& pos = _buffer_pos(buffer.type);
  if (pos == id) {
    GL_CALL(glBindBuffer, buffer.type, NULL_BINDING);
    pos = NULL_BINDING;
  }
  GL_CALL(glDeleteBuffers, 1, &id);
}

bool gl_state::buffer_bind(GLuint id, GLenum type) {
  GLenum& pos = _buffer_pos(type);
  if (pos == id) {
    return false;
  }
  GL_CALL(glBindBuffer, type, id);
  pos = id;
  return true;
}

ctx_buff_status gl_state::buffer_upload(glbuffer_t& buffer, size_t size,
                                        size_t offset, const void* data)
{
  NTF_ASSERT(buffer.flags & GL_DYNAMIC_STORAGE_BIT);
  NTF_ASSERT(size+offset <= buffer.size);

  buffer_bind(buffer.id, buffer.type);
  GL_CALL(glBufferSubData, buffer.type, offset, size, data);
  return CTX_BUFF_STATUS_OK;
}

ctx_buff_status gl_state::buffer_map(glbuffer_t& buffer, void** ptr, size_t size, size_t offset) {
  NTF_ASSERT(ptr);
  NTF_ASSERT(offset+size <= buffer.size);
  NTF_ASSERT(buffer.mapping != 0);

  buffer_bind(buffer.id, buffer.type);
  void* ptr_ret = GL_CALL_RET(glMapBufferRange, buffer.type, offset, size, buffer.mapping);
  *ptr = ptr_ret;
  return CTX_BUFF_STATUS_OK;
}

void gl_state::buffer_unmap(glbuffer_t& buffer, void*) {
  buffer_bind(buffer.id, buffer.type);
  GL_CALL(glUnmapBuffer, buffer.type);
}

ctx_buff_status gl_context::create_buffer(ctx_buff& buff, const ctx_buff_desc& desc) {
  ctx_buff handle = _buffers.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& buffer = _buffers.get(handle);
  const auto status = _state.create_buffer(buffer, desc.type, desc.flags, desc.size, desc.data);
  if (status != CTX_BUFF_STATUS_OK) {
    _buffers.push(handle);
    return status;
  }

  NTF_ASSERT(buffer.id);
  buff = handle;
  return status;
}

ctx_buff_status gl_context::destroy_buffer(ctx_buff buff) noexcept {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  _state.destroy_buffer(buffer);
  _buffers.push(buff);
  return CTX_BUFF_STATUS_OK;
}

ctx_buff_status gl_context::update_buffer(ctx_buff buff, const buffer_data& data) {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  return _state.buffer_upload(buffer, data.size, data.offset, data.data);
}

ctx_buff_status gl_context::map_buffer(ctx_buff buff, void** ptr, size_t size, size_t offset) {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  return _state.buffer_map(buffer, ptr, size, offset);
}

ctx_buff_status gl_context::unmap_buffer(ctx_buff buff, void* ptr) noexcept {
  if (!_buffers.validate(buff) || !ptr) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  _state.buffer_unmap(buffer, ptr);
  return CTX_BUFF_STATUS_OK;
}

} // namespace ntf::render
