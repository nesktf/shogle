#pragma once

#include "./opengl.hpp"

namespace ntf {

class gl_buffer {
private:
  gl_buffer(gl_context& ctx) :
    _ctx(ctx), _id(0) {}

private:
  void load(r_buffer_type type, const void* data, size_t size);
  void unload();

public:
  void data(const void* data, size_t size, size_t offset);

public:
  r_buffer_type type() const { return _type; }
  size_t size() const { return _size; }

private:
  bool complete() const { return _id != 0; }

private:
  gl_context& _ctx;

  GLuint _id;
  r_buffer_type _type;
  size_t _size;
  GLbitfield _alloc_flags;

private:
  friend class gl_context;
};

} // namespace ntf
