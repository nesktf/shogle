#include "./context_private.hpp"
#include <GL/glext.h>
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/context.hpp>

namespace shogle {

namespace {

constexpr gldefs::GLenum WRITE_BUFFER = 0x8F37; // GL_COPY_WRITE_BUFFER

// constexpr gldefs::GLenum READ_BUFFER = 0x8F36;  // GL_COPY_READ_BUFFER

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
  SHOGLE_UNREACHABLE();
#undef STR
}

} // namespace

// Internal constructor
gl_buffer::gl_buffer(create_t, gldefs::GLhandle id, gldefs::GLbitfield usage_flags, size_t size) :
    _id(id), _usage_flags(usage_flags), _size(size) {}

// Inmutable sized constructor with optional data
gl_buffer::gl_buffer(gl_context& gl, size_t size, gldefs::GLbitfield usage_flags,
                     const void* data) :
    gl_buffer(allocate(gl, size, usage_flags, data).value()) {}

auto gl_buffer::_allocate_span(gl_context& gl, gldefs::GLhandle* buffs, size_t count, size_t size,
                               gldefs::GLbitfield usage_flags, const void* data) -> n_err_return {
  SHOGLE_ASSERT(buffs);
  SHOGLE_ASSERT(count);
  GL_ASSERT(glGenBuffers(count, buffs));
  size_t i = 0;
  for (; i < count; ++i) {
    GL_ASSERT(glBindBuffer(WRITE_BUFFER, buffs[i]));
    gldefs::GLenum err = 0;
    err = GL_RET_ERR(glBufferStorage(WRITE_BUFFER, size, data, usage_flags));
    if (err) {
      GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
      GL_ASSERT(glDeleteBuffers(count - i + 1, buffs + i));
      return {i, err};
    }
    SHOGLE_GL_LOG(VERBOSE, "BUFFER_ALLOC ({}) [sz: {}B]", buffs[i], size);
  }
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
  return {i, GL_NO_ERROR};
}

gl_expect<gl_buffer> gl_buffer::allocate(gl_context& gl, size_t size,
                                         gldefs::GLbitfield usage_flags, const void* data) {
  gldefs::GLhandle buff;
  const auto [count, err] = _allocate_span(gl, &buff, 1, size, usage_flags, data);
  if (err) {
    return {unexpect, err};
  }
  SHOGLE_UNUSED(count);
  SHOGLE_ASSERT(count == 1);
  return {in_place, create_t{}, buff, usage_flags, size};
}

void gl_buffer::deallocate(gl_context& gl, gl_buffer& buff) noexcept {
  if (SHOGLE_UNLIKELY(buff._id == GL_NULL_HANDLE)) {
    return;
  }
  GL_ASSERT(glDeleteBuffers(1, &buff._id));
  SHOGLE_GL_LOG(VERBOSE, "BUFFER_DEALLOC ({}) [sz: {}B]", buff._id, buff._size);
  buff._id = GL_NULL_HANDLE;
}

void gl_buffer::deallocate_n(gl_context& gl, gl_buffer* buffs, size_t buff_count) noexcept {
  if (SHOGLE_UNLIKELY(!buffs)) {
    return;
  }
  for (size_t i = 0; i < buff_count; ++i) {
    if (SHOGLE_UNLIKELY(buffs[i]._id == GL_NULL_HANDLE)) {
      continue;
    }
    GL_CALL(glDeleteBuffers(1, &buffs[i]._id));
    SHOGLE_GL_LOG(VERBOSE, "BUFFER_DEALLOC ({}) [sz: {}B]", buffs[i]._id, buffs[i]._size);
    buffs[i]._id = GL_NULL_HANDLE;
  }
}

void gl_buffer::deallocate_n(gl_context& gl, span<gl_buffer> buffs) noexcept {
  deallocate_n(gl, buffs.data(), buffs.size());
}

gl_expect<void> gl_buffer::upload_data(gl_context& gl, const void* data, size_t size,
                                       size_t offset) const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  SHOGLE_ASSERT(data, "Buffer upload with null pointer");
  SHOGLE_ASSERT(size, "Buffer upload with no size");
  SHOGLE_ASSERT(size + offset <= _size, "Buffer upload out of bounds");
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, _id));
  const auto err =
    GL_RET_ERR(glBufferSubData(WRITE_BUFFER, (GLintptr)offset, (GLsizeiptr)size, data));
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
  if (err) {
    return {unexpect, err};
  } else {
    SHOGLE_GL_LOG(VERBOSE, "BUFFER_WRITE ({}) [ptr: {}, sz: {}B/{}B, off: {}B]", _id,
                  fmt::ptr(data), size, _size, offset);
    return {};
  }
}

gl_expect<void> gl_buffer::read_data(gl_context& gl, void* data, size_t size,
                                     size_t offset) const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  SHOGLE_ASSERT(data, "Buffer read with null pointer");
  SHOGLE_ASSERT(size, "Buffer read with no size");
  SHOGLE_ASSERT(size + offset <= _size, "Buffer read out of bounds");
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, _id));
  const auto err =
    GL_RET_ERR(glGetBufferSubData(WRITE_BUFFER, (GLintptr)offset, (GLsizeiptr)size, data));
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
  if (err) {
    return {unexpect, err};
  } else {
    SHOGLE_GL_LOG(VERBOSE, "BUFFER_READ ({}) [ptr: {}, sz: {}B/{}B, off: {}B]", _id,
                  fmt::ptr(data), size, _size, offset);
    return {};
  }
}

gl_expect<void*> gl_buffer::map_range(gl_context& gl, size_t size, size_t offset,
                                      gldefs::GLbitfield access_flags) const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, _id));
  void* ptr = GL_CALL(glMapBufferRange(WRITE_BUFFER, offset, size, access_flags));
  const auto err = gl.get_error();
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
  if (err != GL_NO_ERROR) {
    return {unexpect, err};
  }
  SHOGLE_GL_LOG(VERBOSE, "BUFFER_MAP_RANGE ({}) [ptr: {}, sz: {}B/{}B, off: {}b, flags: {}]", _id,
                fmt::ptr(ptr), size, _size, offset, access_flags);
  SHOGLE_ASSERT(ptr);
  return {in_place, ptr};
}

gl_expect<void*> gl_buffer::map(gl_context& gl, gl_buffer::mapping_access access) const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, _id));
  void* ptr = GL_CALL(glMapBuffer(WRITE_BUFFER, access));
  const auto err = gl.get_error();
  if (err != GL_NO_ERROR) {
    return {unexpect, err};
  }
  SHOGLE_GL_LOG(VERBOSE, "BUFFER_MAP ({}) (ptr: {}, access: {})", _id, fmt::ptr(ptr),
                (gldefs::GLenum)access);
  SHOGLE_ASSERT(ptr);
  return {in_place, ptr};
}

void gl_buffer::unmap(gl_context& gl) const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  SHOGLE_GL_LOG(VERBOSE, "BUFFER_UNMAP ({})", _id);
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, _id));
  GL_ASSERT(glUnmapBuffer(WRITE_BUFFER));
  GL_ASSERT(glBindBuffer(WRITE_BUFFER, GL_DEFAULT_BINDING));
}

gldefs::GLhandle gl_buffer::id() const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  return _id;
}

gldefs::GLbitfield gl_buffer::usage_flags() const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  return _usage_flags;
}

size_t gl_buffer::size() const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_buffer use after free");
  return _size;
}

} // namespace shogle
