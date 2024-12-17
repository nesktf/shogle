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

GLenum gl_buffer_type_cast(r_buffer_type type);

} // namespace ntf
