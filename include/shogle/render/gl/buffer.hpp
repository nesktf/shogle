#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_buffer {
public:
  using context_type = gl_context;

  enum buffer_type : gldefs::GLenum {
    BUFFER_VERTEX = 0x8892,  // GL_ARRAY_BUFFER,
    BUFFER_INDEX = 0x8893,   // GL_ELEMENT_ARRAY_BUFFER,
    BUFFER_UNIFORM = 0x8A11, // GL_UNIFORM_BUFFER,
    BUFFER_SHADER = 0x90D2,  // GL_SHADER_STORAGE_BUFFER,
    BUFFER_TEXTURE = 0x8C2A, // GL_TEXTURE_BUFFER
  };

  enum buffer_bits : gldefs::GLbitfield {
    USAGE_MAP_READ_BIT = 0x0001,        // GL_MAP_READ_BIT,
    USAGE_MAP_WRITE_BIT = 0x0002,       // GL_MAP_WRITE_BIT
    USAGE_MAP_PERSISTENT_BIT = 0x0040,  // GL_MAP_PERSISTENT_BIT
    USAGE_MAP_COHERENT_BIT = 0x0080,    // GL_MAP_COHERENT_BIT,
    USAGE_CLIENT_STORAGE_BIT = 0x0200,  // GL_CLIENT_STORAGE_BIT,
    USAGE_DYNAMIC_STORAGE_BIT = 0x0100, // GL_DYNAMIC_STORAGE_BIT
  };

  static constexpr buffer_bits DEFAULT_USAGE = USAGE_DYNAMIC_STORAGE_BIT;

  /*
  enum buffer_mut_usage : gldefs::GLenum {
  USAGE_STREAM_DRAW = 0x88E0,  // GL_STREAM_DRAW
  USAGE_STREAM_READ = 0x88E1,  // GL_STREAM_READ
  USAGE_STREAM_COPY = 0x88E2,  // GL_STREAM_COPY
  USAGE_STATIC_DRAW = 0x88E4,  // GL_STATIC_DRAW
  USAGE_STATIC_READ = 0x88E5,  // GL_STATIC_READ
  USAGE_STATIC_COPY = 0x88E6,  // GL_STATIC_COPY
  USAGE_DYNAMIC_DRAW = 0x88E8, // GL_DYNAMIC_DRAW
  USAGE_DYNAMIC_READ = 0x88E9, // GL_DYNAMIC_READ
  USAGE_DYNAMIC_COPY = 0x88EA, // GL_DYNAMIC_READ
  };
  */

  enum mapping_access : gldefs::GLbitfield {
    ACCESS_READONLY = 0x88B8,  // GL_READ_ONLY
    ACCESS_WRITEONYL = 0x88B9, // GL_WRITE_ONLY
    ACCESS_READWRITE = 0x88BA, // GL_READ_WRITE
  };

  enum mapping_bits : gldefs::GLbitfield {
    MAP_READ_BIT = 0x0001,              // GL_MAP_READ_BIT
    MAP_WRITE_BIT = 0x0002,             // GL_MAP_WRITE_BIT
    MAP_INVALIDATE_RANGE_BIT = 0x0004,  // GL_MAP_INVALIDATE_RANGE_BIT
    MAP_INVALIDATE_BUFFER_BIT = 0x0008, // GL_MAP_INVALIDATE_BUFFER_BIT
    MAP_FLUSH_EXPLICIT_BIT = 0x0010,    // GL_MAP_FLUSH_EXPLICIT_BIT
    MAP_FLUSH_UNSYNCHRONIZED = 0x0020,  // GL_MAP_UNSYNCHRONIZED_BIT
  };

  using n_err_return = std::pair<u32, gldefs::GLenum>;

private:
  struct create_t {};

  template<typename Cont>
  static constexpr bool emplace_buff_container =
    ntf::meta::growable_emplace_container_of<std::decay_t<Cont>, gl_texture, create_t,
                                             gldefs::GLhandle, buffer_type, size_t, buffer_bits>;

  template<typename Cont>
  static constexpr bool growable_buff_container =
    ntf::meta::growable_push_container_of<std::decay_t<Cont>, gl_buffer> ||
    emplace_buff_container<Cont>;

public:
  // Internal constructor
  gl_buffer(create_t, gldefs::GLhandle id, buffer_type type, size_t size, buffer_bits usage);

  // Sized constructor with optional data
  gl_buffer(gl_context& gl, buffer_type type, size_t size, buffer_bits usage = DEFAULT_USAGE,
            const void* data = nullptr);

private:
  static n_err_return _allocate_span(gl_context& gl, gldefs::GLhandle* buffs, u32 count,
                                     buffer_type type, size_t size, buffer_bits usage,
                                     const void* data);

public:
  static gl_expect<gl_buffer> allocate(gl_context& gl, buffer_type type, size_t size,
                                       buffer_bits usage = DEFAULT_USAGE,
                                       const void* data = nullptr);

  template<typename Cont>
  static n_err_return allocate_n(gl_context& gl, Cont&& cont, u32 count, buffer_type type,
                                 size_t size, buffer_bits usage = DEFAULT_USAGE,
                                 const void* data = nullptr)
  requires(growable_buff_container<Cont>);

  static void deallocate(gl_context& gl, gl_buffer& buff);
  static void deallocate_n(gl_context& gl, gl_buffer* buffs, u32 buff_count);
  static void deallocate_n(gl_context& gl, span<gl_buffer> buffs);

public:
  gl_expect<void> upload_data(gl_context& gl, const void* data, size_t size, size_t offset);
  gl_expect<void> read_data(gl_context& gl, void* data, size_t size, size_t offset);

  gl_expect<void*> map(gl_context& gl, mapping_access access);
  gl_expect<void*> map_range(gl_context& gl, size_t size, size_t offset,
                             mapping_bits access_flags);
  void unmap(gl_context& gl);

public:
  gldefs::GLhandle id() const;
  buffer_type type() const;
  buffer_bits usage_flags() const;
  size_t size() const;
  bool invalidated() const;

public:
  explicit operator bool() const { return !invalidated(); }

private:
  size_t _size;
  gldefs::GLhandle _id;
  buffer_type _type;
  buffer_bits _flags;
};

template<>
struct gl_deleter<gl_buffer> {
  void operator()(gl_context& gl, gl_buffer* buffers, u32 count) noexcept {
    gl_buffer::deallocate_n(gl, buffers, count);
  }

  void operator()(gl_context& gl, gl_buffer& buffer) noexcept {
    gl_buffer::deallocate(gl, buffer);
  }
};

} // namespace shogle

#ifndef SHOGLE_RENDER_GL_BUFFER_INL
#include <shogle/render/gl/buffer.inl>
#endif
