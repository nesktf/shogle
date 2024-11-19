#include "./context.hpp"
#include "../meshes.hpp"

static ntf::gl_context* _current_context{nullptr};

namespace ntf {

gl_context& gl_context::current_context() {
  NTF_ASSERT(_current_context, "Current gl_context is invalid");
  return *_current_context;
}

void gl_context::set_current_context(gl_context& ctx) {
  _current_context = &ctx;
}

void gl_context::destroy() {
  if (this == _current_context) {
    _current_context = nullptr;
  }
}

void gl_context::set_viewport(size_t w, size_t h) {
  glViewport(0, 0, w, h);
}

void gl_context::set_viewport(ivec2 sz) {
  glViewport(0, 0, sz.x, sz.y);
}

void gl_context::set_viewport(size_t x, size_t y, size_t w, size_t h) {
  glViewport(x, y, w, h);
}

void gl_context::set_viewport(ivec2 pos, ivec2 sz) {
  glViewport(pos.x, pos.y, sz.x, sz.y);
}

void gl_context::clear_viewport(color4 color, clear flag) {
  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if ((flag & clear::depth) != clear::none) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  if ((flag & clear::stencil) != clear::none) {
    mask |= GL_STENCIL_BUFFER_BIT;
  }
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(mask);
}

void gl_context::clear_viewport(color3 color, clear flag) {
  clear_viewport(color4{color, 1.0f}, flag);
}

void gl_context::set_stencil_test(bool flag) {
  if (flag) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
}

void gl_context::set_depth_test(bool flag) {
  if (flag) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void gl_context::set_blending(bool flag) {
  if (flag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO: set_blending_fun
  } else {
    glDisable(GL_BLEND);
  }
}

void gl_context::set_depth_fun(depth_fun fun) {
  //TODO: More funcs
  switch (fun) {
    case depth_fun::less: {
      glDepthFunc(GL_LESS);
      break;
    }
    case depth_fun::lequal: {
      glDepthFunc(GL_LEQUAL);
      break;
    }
  }
}

auto gl_context::make_quad() -> mesh {
  mesh out;
  // out.load(
  //   mesh_primitive::triangles,
  //   ntf::quad_vertices, sizeof(ntf::quad_vertices), mesh_buffer::static_draw,
  //   ntf::quad_indices, sizeof(ntf::quad_indices), mesh_buffer::static_draw,
  //   shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec3>{}
  // );
  return out;
}

auto gl_context::make_cube() -> mesh {
  mesh out;
  out.load(
    mesh_primitive::triangles,
    ntf::cube_vertices, sizeof(ntf::cube_vertices), mesh_buffer::static_draw,
    shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec3>{}
  );
  return out;
}

} // namespace ntf
