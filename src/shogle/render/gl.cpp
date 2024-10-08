
#include <shogle/render/gl.hpp>

namespace {

enum mesh_id {
  QUAD = 0,
  CUBE,
  TEXT,
  MESH_COUNT
};

// TODO: Use the mesh class instead of raw calls
struct {
  GLuint VAO{0}, VBO{0}, EBO{0};
  size_t attribs{0};
  size_t draw_count{0};
} meshes[MESH_COUNT];

void create_meshes() {
  // quad mesh
  meshes[QUAD].attribs = 3;
  meshes[QUAD].draw_count = 6;
  glGenVertexArrays(1, &meshes[QUAD].VAO);
  glGenBuffers(1, &meshes[QUAD].VBO);
  glGenBuffers(1, &meshes[QUAD].EBO);
  glBindVertexArray(meshes[QUAD].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, meshes[QUAD].VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ntf::quad_vertices), ntf::quad_vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[QUAD].EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ntf::quad_indices), ntf::quad_indices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
  glBindVertexArray(0);

  // cube mesh
  meshes[CUBE].attribs = 3;
  meshes[CUBE].draw_count = 36;
  glGenVertexArrays(1, &meshes[CUBE].VAO);
  glGenBuffers(1, &meshes[CUBE].VBO);
  glBindVertexArray(meshes[CUBE].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, meshes[CUBE].VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ntf::cube_vertices), ntf::cube_vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
  glBindVertexArray(0);

  // text mesh
  meshes[TEXT].attribs = 1;
  meshes[TEXT].draw_count = 6;
  glGenVertexArrays(1, &meshes[TEXT].VAO);
  glGenBuffers(1, &meshes[TEXT].VBO);
  glBindVertexArray(meshes[TEXT].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, meshes[TEXT].VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, nullptr, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);
}

void destroy_meshes() {
  for (size_t i = 0; i < MESH_COUNT; ++i) {
    auto& m = meshes[i];
    glBindVertexArray(m.VAO);
    for (size_t j = 0; j < meshes[i].attribs; ++j) {
      glDisableVertexAttribArray(j);
    }
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m.VAO);
    glDeleteBuffers(1, &m.VBO);
    if (m.EBO) {
      glDeleteBuffers(1, &m.EBO);
    }
  }
}

} // namespace

namespace ntf {

void gl_renderer::init_meshes() {
  create_meshes();
}

void gl_renderer::destroy() {
  destroy_meshes();
}

const char* gl_renderer::name_str() {
  return "OpenGL";
}

void gl_renderer::start_frame() {

}

void gl_renderer::end_frame() {

}

void gl_renderer::set_viewport(size_t w, size_t h) {
  glViewport(0, 0, w, h);
}

void gl_renderer::set_viewport(ivec2 sz) {
  glViewport(0, 0, sz.x, sz.y);
}

void gl_renderer::set_viewport(size_t x, size_t y, size_t w, size_t h) {
  glViewport(x, y, w, h);
}

void gl_renderer::set_viewport(ivec2 pos, ivec2 sz) {
  glViewport(pos.x, pos.y, sz.x, sz.y);
}

void gl_renderer::clear_viewport(color4 color, clear flag) {
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

void gl_renderer::clear_viewport(color3 color, clear flag) {
  clear_viewport(color4{color, 1.0f}, flag);
}

void gl_renderer::set_stencil_test(bool flag) {
  if (flag) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
}

void gl_renderer::set_depth_test(bool flag) {
  if (flag) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void gl_renderer::set_blending(bool flag) {
  if (flag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO: set_blending_fun
  } else {
    glDisable(GL_BLEND);
  }
}

void gl_renderer::set_depth_fun(depth_fun fun) {
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

void gl_renderer::draw_quad() {
  glBindVertexArray(meshes[QUAD].VAO);
  glDrawElements(GL_TRIANGLES, meshes[QUAD].draw_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void gl_renderer::draw_cube() {
  glBindVertexArray(meshes[CUBE].VAO);
  glDrawArrays(GL_TRIANGLES, 0, meshes[CUBE].draw_count);
  glBindVertexArray(0);
}

void gl_renderer::draw_text(const font_atlas& atlas, vec2 pos, float scale, std::string_view text) {
  glBindVertexArray(meshes[TEXT].VAO);

  float x = pos.x, y = pos.y;
  std::string_view::const_iterator c;
  for (c = text.begin(); c != text.end(); ++c) {
    auto [tex, ch] = atlas.at(*c);

    float xpos = x + ch.bearing.x*scale;
    float ypos = y - ch.bearing.y*scale;

    float w = ch.size.x*scale;
    float h = ch.size.y*scale;

    float vert[6][4] {
      { xpos,     ypos + h, 0.0f, 1.0f },
      { xpos,     ypos,     0.0f, 0.0f },
      { xpos + w, ypos,     1.0f, 0.0f },

      { xpos,     ypos + h, 0.0f, 1.0f },
      { xpos + w, ypos,     1.0f, 0.0f },
      { xpos + w, ypos + h, 1.0f, 1.0f }
    };
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindBuffer(GL_ARRAY_BUFFER, meshes[TEXT].VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert), vert);

    glDrawArrays(GL_TRIANGLES, 0, meshes[TEXT].draw_count);

    x += (ch.advance >> 6)*scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace ntf
