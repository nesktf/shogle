#include "./context_private.hpp"
#include <GL/glext.h>
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/context.hpp>

namespace shogle {

namespace {
std::string_view buffer_type_name(gl_buffer::buffer_type type) {
#define STR(enum_)              \
  case gl_buffer::TYPE_##enum_: \
    return #enum_
  switch (type) {
    STR(VERTEX);
    STR(INDEX);
    STR(SHADER);
    STR(UNIFORM);
    STR(TEXTURE);
    default:
      return "UNKNOWN";
  }
  NTF_UNREACHABLE();
#undef STR
}
} // namespace

gl_buffer::gl_buffer(create_t, gldefs::GLhandle id, buffer_type type, size_t size,
                     gldefs::GLenum usage) : _size(size), _id(id), _type(type), _usage(usage) {}

gl_buffer::gl_buffer(gl_context& gl, buffer_type type, size_t size, gldefs::GLbitfield usage_flags,
                     const void* data) :
    gl_buffer(::shogle::gl_buffer::allocate(gl, type, size, usage_flags, data).value()) {}

gl_buffer::gl_buffer(mutable_tag, gl_context& gl, buffer_type type, size_t size,
                     buffer_mut_usage usage, const void* data) :
    gl_buffer(::shogle::gl_buffer::allocate_mut(gl, type, size, usage, data).value()) {}

auto gl_buffer::_allocate_span(gl_context& gl, gldefs::GLhandle* buffs, size_t count,
                               buffer_type type, size_t size, gldefs::GLenum usage,
                               const void* data, bool is_mutable) -> n_err_return {
  NTF_ASSERT(buffs);
  NTF_ASSERT(count);
  GL_ASSERT(glGenBuffers(count, buffs));
  size_t i = 0;
  for (; i < count; ++i) {
    GL_ASSERT(glBindBuffer(type, buffs[i]));
    gldefs::GLenum err = 0;
    if (is_mutable) {
      err = GL_RET_ERR(glBufferData(type, size, data, usage));
    } else {
      err = GL_RET_ERR(glBufferStorage(type, size, data, usage));
    }
    if (err) {
      GL_ASSERT(glBindBuffer(type, GL_DEFAULT_BINDING));
      GL_ASSERT(glDeleteBuffers(count - i + 1, buffs + i));
      return {i, err};
    }
    SHOGLE_GL_LOG(verbose, "BUFFER_ALLOC ({}) (sz: {}B, type: {}, mut: {})", buffs[i], size,
                  buffer_type_name(type), is_mutable);
  }
  // GL_ASSERT(glBindBuffer(type, buff))
  GL_ASSERT(glBindBuffer(type, GL_DEFAULT_BINDING));
  return {i, GL_NO_ERROR};
}

gl_expect<gl_buffer> gl_buffer::allocate(gl_context& gl, buffer_type type, size_t size,
                                         gldefs::GLbitfield usage_flags, const void* data) {
  gldefs::GLhandle buff;
  const auto [count, err] =
    _allocate_span(gl, &buff, 1, type, size, (gldefs::GLenum)usage_flags, data, false);
  if (err) {
    return {ntf::unexpect, err};
  }
  NTF_UNUSED(count);
  NTF_ASSERT(count == 1);
  return {ntf::in_place, create_t{}, buff, type, size, (gldefs::GLenum)usage_flags};
}

gl_expect<gl_buffer> gl_buffer::allocate_mut(gl_context& gl, buffer_type type, size_t size,
                                             buffer_mut_usage usage, const void* data) {
  gldefs::GLhandle buff;
  const auto [count, err] =
    _allocate_span(gl, &buff, 1, type, size, (gldefs::GLenum)usage, data, true);
  if (err) {
    return {ntf::unexpect, err};
  }
  NTF_UNUSED(count);
  NTF_ASSERT(count == 1);
  return {ntf::in_place, create_t{}, buff, type, size, (gldefs::GLenum)usage};
}

void gl_buffer::deallocate(gl_context& gl, gl_buffer& buff) noexcept {
  if (NTF_UNLIKELY(buff.invalidated())) {
    return;
  }
  GL_ASSERT(glDeleteBuffers(1, &buff._id));
  SHOGLE_GL_LOG(verbose, "BUFFER_DEALLOC ({}) (sz: {}B, type: {})", buff._id, buff._size,
                buffer_type_name(buff._type));
  buff._id = GL_NULL_HANDLE;
}

void gl_buffer::deallocate_n(gl_context& gl, gl_buffer* buffs, size_t buff_count) noexcept {
  if (NTF_UNLIKELY(!buffs)) {
    return;
  }
  for (size_t i = 0; i < buff_count; ++i) {
    if (NTF_UNLIKELY(buffs[i].invalidated())) {
      continue;
    }
    GL_CALL(glDeleteBuffers(1, &buffs[i]._id));
    SHOGLE_GL_LOG(verbose, "BUFFER_DEALLOC ({}) (sz: {}B, type: {})", buffs[i]._id, buffs[i]._size,
                  buffer_type_name(buffs[i]._type));
    buffs[i]._id = GL_NULL_HANDLE;
  }
}

void gl_buffer::deallocate_n(gl_context& gl, span<gl_buffer> buffs) noexcept {
  deallocate_n(gl, buffs.data(), buffs.size());
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
    SHOGLE_GL_LOG(verbose, "BUFFER_WRITE ({}), (ptr: {}, sz: {}B/{}B, off: {}B)", _id,
                  fmt::ptr(data), size, _size, offset);
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
    SHOGLE_GL_LOG(verbose, "BUFFER_READ ({}) (ptr: {}, sz: {}B/{}B, off: {}B)", _id,
                  fmt::ptr(data), size, _size, offset);
    return {};
  }
}

gl_expect<void*> gl_buffer::map_range(gl_context& gl, size_t size, size_t offset,
                                      gldefs::GLbitfield access_flags) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(_type, _id));
  void* ptr = GL_CALL(glMapBufferRange(_type, offset, size, access_flags));
  const auto err = gl.get_error();
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  SHOGLE_GL_LOG(verbose, "BUFFER_MAP_RANGE ({}) (ptr: {}, sz: {}B/{}B, off: {}b, flags: {})", _id,
                fmt::ptr(ptr), size, _size, offset, access_flags);
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

gl_expect<void*> gl_buffer::map(gl_context& gl, gl_buffer::mapping_access access) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(_type, _id));
  void* ptr = GL_CALL(glMapBuffer(_type, access));
  const auto err = gl.get_error();
  if (err != GL_NO_ERROR) {
    return {ntf::unexpect, err};
  }
  SHOGLE_GL_LOG(verbose, "BUFFER_MAP ({}) (ptr: {}, access: {})", _id, fmt::ptr(ptr),
                (gldefs::GLenum)access);
  NTF_ASSERT(ptr);
  return {ntf::in_place, ptr};
}

void gl_buffer::unmap(gl_context& gl) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  SHOGLE_GL_LOG(verbose, "BUFFER_UNMAP ({})", _id);
  GL_ASSERT(glBindBuffer(_type, _id));
  GL_ASSERT(glUnmapBuffer(_type));
  GL_ASSERT(glBindBuffer(_type, GL_DEFAULT_BINDING));
}

void gl_buffer::mut_realloc(gl_context& gl, size_t size, buffer_mut_usage usage,
                            const void* data) {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  NTF_ASSERT(is_mutable(), "Can't reallocate inmutable buffer");
  GL_ASSERT(glBufferData(_id, size, data, _usage));
  SHOGLE_GL_LOG(verbose, "BUFFER_REALLOC ({}) (data: {}, sz: {}B -> {}B, usage: {})", _id,
                fmt::ptr(data), _size, size, (gldefs::GLenum)usage);
  _size = size;
  _usage = (gldefs::GLenum)usage;
}

gldefs::GLhandle gl_buffer::id() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _id;
}

gl_buffer::buffer_type gl_buffer::type() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _type;
}

size_t gl_buffer::size() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  return _size;
}

gldefs::GLbitfield gl_buffer::usage_flags() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  NTF_ASSERT(!is_mutable(), "Mutable buffer doesn't have usage bits");
  return (gldefs::GLbitfield)_usage;
}

gl_buffer::buffer_mut_usage gl_buffer::mut_usage() const {
  NTF_ASSERT(!invalidated(), "gl_buffer use after free");
  NTF_ASSERT(is_mutable(), "Inmutable buffer doesn't have usage state");
  return (buffer_mut_usage)_usage;
}

bool gl_buffer::is_mutable() const noexcept {
  // Every mutable enum starts with bit pattern 0x8000
  return _usage & 0x8000;
}

bool gl_buffer::invalidated() const noexcept {
  return _id == GL_NULL_HANDLE;
}

} // namespace shogle
