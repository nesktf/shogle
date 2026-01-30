#include "./context_private.hpp"
#include <GL/glext.h>
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/context.hpp>

namespace shogle {

gl_buffer::gl_buffer(create_t, gldefs::GLhandle id, buffer_type type, size_t size,
                     buffer_bits usage) : _size(size), _id(id), _type(type), _flags(usage) {}

gl_buffer::gl_buffer(gl_context& gl, buffer_type type, size_t size, buffer_bits usage,
                     const void* data) :
    gl_buffer(::shogle::gl_buffer::allocate(gl, type, size, usage, data).value()) {}

auto gl_buffer::_allocate_span(gl_context& gl, gldefs::GLhandle* buffs, u32 count,
                               buffer_type type, size_t size, buffer_bits usage, const void* data)
  -> n_err_return {
  NTF_ASSERT(buffs);
  NTF_ASSERT(count);
  GL_ASSERT(glGenBuffers(count, buffs));
  u32 i = 0;
  for (; i < count; ++i) {
    GL_ASSERT(glBindBuffer(type, buffs[i]));
    const auto err = GL_RET_ERR(glBufferStorage(type, size, data, usage));
    if (err) {
      GL_ASSERT(glBindBuffer(type, GL_DEFAULT_BINDING));
      GL_ASSERT(glDeleteBuffers(count - i - 1, buffs + i));
      return {i, err};
    }
  }
  // GL_ASSERT(glBindBuffer(type, buff))
  GL_ASSERT(glBindBuffer(type, GL_DEFAULT_BINDING));
  return {i, GL_NO_ERROR};
}

gl_expect<gl_buffer> gl_buffer::allocate(gl_context& gl, buffer_type type, size_t size,
                                         buffer_bits usage, const void* data) {
  gldefs::GLhandle buff;
  const auto [count, err] = _allocate_span(gl, &buff, 1, type, size, usage, data);
  if (err) {
    return {ntf::unexpect, err};
  }
  NTF_UNUSED(count);
  NTF_ASSERT(count == 1);
  return {ntf::in_place, create_t{}, buff, type, size, usage};
}

void gl_buffer::deallocate(gl_context& gl, gl_buffer& buff) {
  NTF_ASSERT(!buff.invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(buff.type(), buff.id()));
  GL_ASSERT(glDeleteBuffers(1, &buff._id));
  GL_ASSERT(glBindBuffer(buff.type(), GL_DEFAULT_BINDING));
  buff._id = GL_NULL_HANDLE;
}

void gl_buffer::deallocate_n(gl_context& gl, gl_buffer* buffs, u32 buff_count) {
  if (!buffs) {
    return;
  }
  for (u32 i = 0; i < buff_count; ++i) {
    NTF_ASSERT(!buffs[i].invalidated(), "gl_buffer use after free");
    GL_ASSERT(glBindBuffer(buffs[i].type(), buffs[i].id()));
    GL_ASSERT(glDeleteBuffers(1, &buffs[i]._id));
    GL_ASSERT(glBindBuffer(buffs[i].type(), GL_DEFAULT_BINDING));
    buffs[i]._id = GL_NULL_HANDLE;
  }
}

void gl_buffer::deallocate_n(gl_context& gl, span<gl_buffer> buffs) {
  for (auto& buff : buffs) {
    NTF_ASSERT(!buff.invalidated(), "gl_buffer use after free");
    GL_ASSERT(glBindBuffer(buff.type(), buff.id()));
    GL_ASSERT(glDeleteBuffers(1, &buff._id));
    GL_ASSERT(glBindBuffer(buff.type(), GL_DEFAULT_BINDING));
    buff._id = GL_NULL_HANDLE;
  }
}

gl_expect<void> gl_buffer::upload_data(gl_context& gl, const void* data, size_t size,
                                       size_t offset) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  NTF_ASSERT(data, "Buffer upload with null pointer");
  NTF_ASSERT(size, "Buffer upload with no size");
  NTF_ASSERT(size + offset <= _size, "Buffer upload out of bounds");
  GL_ASSERT(glBindBuffer(_type, _id));
  const auto err = GL_RET_ERR(glBufferSubData(_type, (GLintptr)offset, (GLsizeiptr)size, data));
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
  if (err) {
    return {ntf::unexpect, err};
  } else {
    return {};
  }
}

gl_expect<void> gl_buffer::read_data(gl_context& gl, void* data, size_t size, size_t offset) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  NTF_ASSERT(data, "Buffer read with null pointer");
  NTF_ASSERT(size, "Buffer read with no size");
  NTF_ASSERT(size + offset <= _size, "Buffer read out of bounds");
  GL_ASSERT(glBindBuffer(_type, _id));
  const auto err = GL_RET_ERR(glGetBufferSubData(_type, (GLintptr)offset, (GLsizeiptr)size, data));
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
  if (err) {
    return {ntf::unexpect, err};
  } else {
    return {};
  }
}

gl_expect<void*> gl_buffer::map_range(gl_context& gl, size_t size, size_t offset,
                                      gl_buffer::mapping_bits access_flags) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(_type, _id));
  void* ptr = GL_CALL(glMapBufferRange((GLenum)_type, offset, size, (GLbitfield)access_flags));
  const auto err = ::shogle::impl::gl_get_error(gl);
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

gl_expect<void*> gl_buffer::map(gl_context& gl, gl_buffer::mapping_access access) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(_type, _id));
  void* ptr = GL_CALL(glMapBuffer(_type, access));
  const auto err = ::shogle::impl::gl_get_error(gl);
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

void gl_buffer::unmap(gl_context& gl) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(_type, _id));
  GL_ASSERT(glUnmapBuffer(_type));
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
}

gldefs::GLhandle gl_buffer::id() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _id;
}

gl_buffer::buffer_type gl_buffer::type() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _type;
}

gl_buffer::buffer_bits gl_buffer::usage_flags() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _flags;
}

size_t gl_buffer::size() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _size;
}

bool gl_buffer::invalidated() const {
  return _id == GL_NULL_HANDLE;
}

} // namespace shogle
