#include "buffer.hpp"

namespace ntf {

void gl_buffer::load(void* data, std::size_t size, buffer_type buffer, storage_type storage) {
  NTF_ASSERT(buffer != buffer_type::none);
  NTF_ASSERT(storage != storage_type::none);
  NTF_ASSERT(!_id);

  glGenBuffers(1, &_id);

  GLenum buff = static_cast<GLenum>(buffer);
  glBindBuffer(buff, _id);
  glBufferData(buff, size, data, static_cast<GLint>(storage));
  glBindBuffer(buff, 0);

  if (_id) {
    _buffer = buffer;
    _storage = storage;
    _size = size;
  }
}

void gl_buffer::update(void* data, std::size_t size, std::size_t offset) {
  NTF_ASSERT(_id);

  GLenum buff = static_cast<GLenum>(_buffer);
  glBindBuffer(buff, _id);
  glBufferSubData(buff, offset, size, data);
  glBindBuffer(buff, 0);
}

void gl_buffer::destroy() {
  NTF_ASSERT(_id);

  glDeleteBuffers(1, &_id);
  _id = 0;
  _size = 0;
  _buffer = buffer_type::none;
  _storage = storage_type::none;
}

void gl_vertex_array::load(gl_buffer& vertex, const std::vector<r_attrib_type>& attribs) {
  NTF_ASSERT(!_id);
  NTF_ASSERT(vertex.id() && vertex.buffer() == gl_buffer::buffer_type::vertex);

  glGenVertexArrays(1, &_id);

  glBindVertexArray(_id);
  glBindBuffer(static_cast<GLenum>(vertex.buffer()), vertex.id());

  _setup_attribs(attribs);

  glBindVertexArray(0);

  if (_id) {
    _vertex = vertex.id();
  }
}

void gl_vertex_array::load(gl_buffer& vertex, gl_buffer& index, 
                           const std::vector<r_attrib_type>& attribs) {
  NTF_ASSERT(!_id);
  NTF_ASSERT(vertex.id() && vertex.buffer() == gl_buffer::buffer_type::vertex);
  NTF_ASSERT(index.id() && index.buffer() == gl_buffer::buffer_type::index);

  glGenVertexArrays(1, &_id);

  glBindVertexArray(_id);
  glBindBuffer(static_cast<GLenum>(vertex.buffer()), vertex.id());
  glBindBuffer(static_cast<GLenum>(index.buffer()), index.id());

  _setup_attribs(attribs);

  glBindVertexArray(0);

  if (_id) {
    _vertex = vertex.id();
    _index = index.id();
  }
}

void gl_vertex_array::destroy() {
  NTF_ASSERT(_id);

  glDeleteVertexArrays(1, &_id);
  _id = 0;
  _vertex = 0;
  _index = 0;
}

void gl_vertex_array::_setup_attribs(const std::vector<r_attrib_type>& attribs) {
  NTF_ASSERT(attribs.size());

  std::size_t attrib_stride = 0;
  for (const auto& attrib : attribs) {
    attrib_stride += r_attrib_size(attrib);
  }

  std::size_t i = 0, offset = 0;
  for (const auto& attrib : attribs) {
    const uint type_count = r_attrib_dim(attrib);
    NTF_ASSERT(type_count <= 4 && type_count >= 1);

    const gl_builtin_type type = gl_attrib_type(attrib);
    NTF_ASSERT(type != gl_builtin_type::none);

    if (type == gl_builtin_type::f64) {
      glVertexAttribLPointer(
        i,
        type_count,
        static_cast<GLenum>(type), // GL_DOUBLE
        attrib_stride,
        reinterpret_cast<void*>(offset)
      );
    } else {
      glVertexAttribPointer(
        i,
        type_count,
        static_cast<GLenum>(type), // GL_FLOAT or GL_INT
        GL_FALSE, // normalize it yourself before submitting
        attrib_stride,
        reinterpret_cast<void*>(offset)
      );
    }

    offset += type_count*r_attrib_size(attrib);
    glEnableVertexAttribArray(i++);
  }
  NTF_ASSERT(offset == attrib_stride);
}

} // namespace ntf
