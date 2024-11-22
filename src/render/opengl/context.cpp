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

auto gl_context::make_quad(mesh_buffer vert_buff, mesh_buffer ind_buff) -> mesh {
  return mesh{}
    .vertices(&ntf::quad_vertices[0], sizeof(ntf::quad_vertices), vert_buff)
    .indices(&ntf::quad_indices[0], sizeof(ntf::quad_indices), ind_buff)
    .attributes(
      shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec3>{}
    );
}

auto gl_context::make_cube(mesh_buffer vert_buff, mesh_buffer) -> mesh {
  return mesh{}
    .vertices(&ntf::cube_vertices[0], sizeof(ntf::cube_vertices), vert_buff)
    .attributes(
      shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec3>{}
    );
}

void gl_context::draw(mesh_primitive prim, const mesh& mesh, std::size_t offset, uint count) {
  NTF_ASSERT(mesh.valid());
  const auto glprim = enumtogl(prim);

  glBindVertexArray(mesh.vao());
  if (mesh.has_indices()) {
    // indices in glDrawElements is an offset in the current EBO
    // it advances offset*sizeof(unsigned int) in the buffer
    glDrawElements(glprim, count > 0 ? count : mesh.elem_count(), GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(offset));
  } else {
    glDrawArrays(glprim, offset, count > 0 ? count : mesh.elem_count());
  }
  glBindVertexArray(0);
}

void gl_context::draw_instanced(mesh_primitive prim, const mesh& mesh, uint primcount,
                               std::size_t offset, uint count) {
  NTF_ASSERT(mesh.valid());

  const auto glprim = enumtogl(prim);
  glBindVertexArray(mesh.vao());
  if (mesh.has_indices()) {
    // indices in glDrawElementsInstanced is also an offset in the current EBO
    // it advances offset*sizeof(unsigned int) in the buffer
    glDrawElementsInstanced(glprim, count > 0 ? count : mesh.elem_count(), GL_UNSIGNED_INT,
                            reinterpret_cast<void*>(offset), primcount);
  } else {
    glDrawArraysInstanced(glprim, offset, count > 0 ? count : mesh.elem_count(), primcount);
  }
  glBindVertexArray(0);
}

} // namespace ntf
