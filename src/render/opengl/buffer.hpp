#pragma once

#include "opengl.hpp"

namespace ntf {

class gl_buffer {
private:
  gl_buffer(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(r_buffer_type type, const void* data, size_t size);
  void unload();

public:
  void data(const void* data, size_t size, size_t offset);

public:
  r_buffer_type type() const { return _type; }
  size_t size() const { return _size; }

private:
  gl_context& _ctx;

  GLuint _id{0};
  r_buffer_type _type{r_buffer_type::none};
  size_t _size{0};
  GLbitfield _alloc_flags{0};

public:
  NTF_DISABLE_MOVE_COPY(gl_buffer);

private:
  friend class gl_context;
};

constexpr inline GLenum gl_buffer_type_cast(r_buffer_type type) {
    switch(type) {
    case r_buffer_type::index:    return GL_ARRAY_BUFFER;
    case r_buffer_type::vertex:   return GL_ELEMENT_ARRAY_BUFFER;
    case r_buffer_type::uniform:  return GL_UNIFORM_BUFFER;

    case r_buffer_type::none:     return 0;
  };
  return 0;
}

} // namespace ntf
