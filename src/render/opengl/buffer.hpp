#pragma once

#include "common.hpp"

#include <vector>

namespace ntf {

enum class gl_buffer_type {
  vertex = GL_ARRAY_BUFFER,
  index = GL_ELEMENT_ARRAY_BUFFER,
  uniform = GL_UNIFORM_BUFFER,
};

enum class gl_buffer_storage {
  static_draw = GL_STATIC_DRAW,
  dynamic_draw = GL_DYNAMIC_DRAW,
  stream_draw = GL_STREAM_DRAW,
};

enum class gl_data_type {
  none = GL_NONE,

  s8  = GL_BYTE,
  u8  = GL_UNSIGNED_BYTE,

  s16 = GL_SHORT,
  u16 = GL_UNSIGNED_SHORT,

  s32 = GL_INT,
  u32 = GL_UNSIGNED_INT,

  f16 = GL_HALF_FLOAT,
  f32 = GL_FLOAT,
  f64 = GL_DOUBLE,
};

static inline gl_data_type shader_data_gl_type(shader_data_type type) {
  switch (type) {
    case shader_data_type::f32:   [[fallthrough]];
    case shader_data_type::vec2:  [[fallthrough]];
    case shader_data_type::vec3:  [[fallthrough]];
    case shader_data_type::vec4:  [[fallthrough]];
    case shader_data_type::mat3:  [[fallthrough]];
    case shader_data_type::mat4:  return gl_data_type::f32;

    case shader_data_type::f64:   [[fallthrough]];
    case shader_data_type::dvec2: [[fallthrough]];
    case shader_data_type::dvec3: [[fallthrough]];
    case shader_data_type::dvec4: [[fallthrough]];
    case shader_data_type::dmat3: [[fallthrough]];
    case shader_data_type::dmat4: return gl_data_type::f64;

    case shader_data_type::i32:   [[fallthrough]];
    case shader_data_type::ivec2: [[fallthrough]];
    case shader_data_type::ivec3: [[fallthrough]];
    case shader_data_type::ivec4: return gl_data_type::s32;

    case shader_data_type::none:  break;
  };

  return gl_data_type::none;
}


using gl_id = GLuint;

class gl_buffer {
public:
  gl_buffer() = default;

public:
  void load(gl_buffer_type buffer, void* data, std::size_t size, gl_buffer_storage storage) {
    NTF_ASSERT(!_id);

    glGenBuffers(1, &_id);

    GLenum buff = static_cast<GLenum>(buffer);
    glBindBuffer(buff, _id);
    glBufferData(buff, size, data, static_cast<GLint>(storage));
    glBindBuffer(buff, 0);

    _buffer = buffer;
    _size = size;
  }

  void update(void* data, std::size_t size, std::size_t offset) {
    NTF_ASSERT(_id);

    GLenum buff = static_cast<GLenum>(_buffer);
    glBindBuffer(buff, _id);
    glBufferSubData(buff, offset, size, data);
    glBindBuffer(buff, 0);
  }

  void destroy() {
    NTF_ASSERT(_id);

    glDeleteBuffers(1, &_id);
    _id = 0;
    _size = 0;
  }

public:
  gl_id id() const { return _id; }
  gl_buffer_type buffer_type() const { return _buffer; }

private:
  gl_buffer_type _buffer{};
  gl_id _id{0};
  std::size_t _size{0};
};

class gl_vertex_array {
public:
  gl_vertex_array() = default;

public:
  void setup_attribs(const std::vector<shader_data_type>& attribs) {
    NTF_ASSERT(attribs.size());

    std::size_t attrib_stride = 0;
    for (const auto& attrib : attribs) {
      attrib_stride += shader_data_size(attrib);
    }

    std::size_t i = 0, offset = 0;
    for (const auto& attrib : attribs) {
      const uint type_count = shader_data_count(attrib);
      NTF_ASSERT(type_count <= 4 && type_count >= 1);

      const gl_data_type gl_type = shader_data_gl_type(attrib);
      NTF_ASSERT(gl_type != gl_data_type::none);

      if (gl_type == gl_data_type::f64) {
        glVertexAttribLPointer(
          i,
          type_count,
          static_cast<GLenum>(gl_type), // GL_DOUBLE
          attrib_stride,
          reinterpret_cast<void*>(offset)
        );
      } else {
        glVertexAttribPointer(
          i,
          type_count,
          static_cast<GLenum>(gl_type), // GL_FLOAT or GL_INT
          GL_FALSE, // normalize it yourself before submitting
          attrib_stride,
          reinterpret_cast<void*>(offset)
        );
      }

      offset += type_count*shader_data_size(attrib);
      glEnableVertexAttribArray(i++);
    }
    NTF_ASSERT(offset == attrib_stride);
  }

  void load(gl_buffer& vertex, const std::vector<shader_data_type>& attribs) {
    NTF_ASSERT(!_id);
    NTF_ASSERT(vertex.id());
    NTF_ASSERT(vertex.buffer_type() == gl_buffer_type::vertex);

    glGenVertexArrays(1, &_id);

    glBindVertexArray(_id);
    glBindBuffer(static_cast<GLenum>(vertex.buffer_type()), vertex.id());

    setup_attribs(attribs);

    glBindVertexArray(0);

    _vertex = vertex.id();
  }

  void load(gl_buffer& vertex, gl_buffer& index, const std::vector<shader_data_type>& attribs) {
    NTF_ASSERT(!_id);
    NTF_ASSERT(vertex.id() && index.id());
    NTF_ASSERT(vertex.buffer_type() == gl_buffer_type::vertex);
    NTF_ASSERT(index.buffer_type() == gl_buffer_type::index);

    glGenVertexArrays(1, &_id);

    glBindVertexArray(_id);
    glBindBuffer(static_cast<GLenum>(vertex.buffer_type()), vertex.id());
    glBindBuffer(static_cast<GLenum>(index.buffer_type()), index.id());

    setup_attribs(attribs);

    glBindVertexArray(0);

    _vertex = vertex.id();
    _index = index.id();
  }

  void destroy() {
    NTF_ASSERT(_id);
    glDeleteVertexArrays(1, &_id);
    _id = 0;
    _vertex = 0;
    _index = 0;
  }

public:
  gl_id vertex() const { return _vertex; }
  gl_id index() const { return _index; }
  gl_id id() const { return _index; }

private:
  gl_id _vertex{0}, _index{0};
  gl_id _id{0};
};

} // namespace ntf
