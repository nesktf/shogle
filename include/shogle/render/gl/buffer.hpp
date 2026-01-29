#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_buffer {
public:
  static constexpr gldefs::GLhandle NULL_BINDING = 0u;

  enum buffer_type : gldefs::GLenum {
    BUFFER_VERTEX = 0x8892,  // GL_ARRAY_BUFFER,
    BUFFER_INDEX = 0x8893,   // GL_ELEMENT_ARRAY_BUFFER,
    BUFFER_UNIFORM = 0x8A11, // GL_UNIFORM_BUFFER,
    BUFFER_SHADER = 0x90D2,  // GL_SHADER_STORAGE_BUFFER,
  };

  enum buffer_bits : gldefs::GLbitfield {
    USAGE_MAP_READ_BIT = 0x0001,        // GL_MAP_READ_BIT,
    USAGE_MAP_WRITE_BIT = 0x0002,       // GL_MAP_WRITE_BIT
    USAGE_MAP_PERSISTENT_BIT = 0x0040,  // GL_MAP_PERSISTENT_BIT
    USAGE_MAP_COHERENT_BIT = 0x0080,    // GL_MAP_COHERENT_BIT,
    USAGE_CLIENT_STORAGE_BIT = 0x0200,  // GL_CLIENT_STORAGE_BIT,
    USAGE_DYNAMIC_STORAGE_BIT = 0x0100, // GL_DYNAMIC_STORAGE_BIT
  };

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

public:
  gl_buffer(gl_context& gl, gldefs::GLhandle id, buffer_type type, gldefs::GLbitfield flags,
            size_t size);
  gl_buffer(gl_context& gl, buffer_type type, buffer_mut_usage usage, const void* data,
            size_t size);

public:
  static gl_sv_expect<gl_buffer> create(gl_context& gl, gl_buffer::buffer_type type,
                                        gl_buffer::buffer_bits usage, const void* data,
                                        size_t size);
  static gl_sv_expect<gl_buffer> create_mutable(gl_context& gl, gl_buffer::buffer_type type,
                                                gl_buffer::buffer_mut_usage usage,
                                                const void* data, size_t size);
  void destroy();
  void rebind_context(gl_context& gl);

public:
  void upload_data(const void* data, size_t size, size_t offset);
  void read_data(void* data, size_t size, size_t offset);

  gl_expect<void*> map(gl_buffer::mapping_access access);
  gl_expect<void*> map_range(size_t size, size_t offset, gl_buffer::mapping_bits access_flags);
  void unmap();

  gl_buffer& set_usage(gl_buffer::buffer_mut_usage usage);
  void reallocate(const void* data, size_t size);

public:
  gldefs::GLhandle id() const;
  buffer_type type() const;
  u32 properties() const;
  size_t size() const;
  bool is_mutable() const;
  gl_context& context() const;

private:
  ref_view<gl_context> gl;
  gldefs::GLhandle _id;
  buffer_type _type;
  u32 _props;
  size_t _size;
};

gl_sv_expect<gl_buffer> gl_make_buffer(gl_context& gl, gl_buffer::buffer_type type, size_t size,
                                       const void* data = nullptr);

} // namespace shogle
