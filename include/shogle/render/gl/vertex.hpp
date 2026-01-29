#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_vertex_layout {
public:
  // I'm too lazy to manage a growing array here
  static constexpr u32 MAX_ATTRIBUTE_BINDINGS = 16;

  struct attribute_binding {
    attribute_type type;
    u32 location;
    size_t offset;
    size_t stride;
  };

  using attribute_array = std::array<attribute_binding, MAX_ATTRIBUTE_BINDINGS>;

private:
  struct create_t {};

public:
  gl_vertex_layout(create_t, gl_context& gl, gldefs::GLhandle vao, attribute_array attributes,
                   u32 attribute_count);
  gl_vertex_layout(gl_context& gl, span<const attribute_binding> binds);

public:
  static gl_sv_expect<gl_vertex_layout> create(gl_context& gl,
                                               span<const attribute_binding> binds);
  void destroy();
  void rebind_context(gl_context& gl);

public:
  gldefs::GLhandle id() const;
  span<const attribute_binding> attributes() const;
  gl_context& context() const;

private:
  attribute_array _attributes;
  ref_view<gl_context> _gl;
  gldefs::GLhandle _vao;
  u32 _attribute_count;
};

} // namespace shogle
