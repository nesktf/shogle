#include <shogle/render/render.hpp>

#include <shogle/render/font.hpp>

static float cube_vert[] {
  // coord               // normal             // texcoord
  -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

   0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

  -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f
};

static float quad_vert[] {
  // coord              //normals           // texcoord   // inv texcoord
  -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   /* 0.0f, 1.0f, // + vec2{0.0f, 1.0f} */
   0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   /* 1.0f, 1.0f, // + vec2{0.0f, 1.0f} */
   0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   /* 1.0f, 0.0f, // + vec2{0.0f, -1.0f} */
  -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,   /* 0.0f, 0.0f, // + vec2{0.0f, -1.0f} */
};

static uint quad_ind[] {
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
};

enum mesh_id {
  QUAD = 0,
  CUBE,
  TEXT,
  MESH_COUNT,
};

static struct {
  GLuint VAO{0}, VBO{0}, EBO{0};
  size_t attribs{0};
  size_t draw_count{0};
} meshes[MESH_COUNT];


namespace ntf::shogle {

bool __render_init(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    return false;
  }

  // quad mesh
  meshes[QUAD].attribs = 3;
  meshes[QUAD].draw_count = 6;
  glGenVertexArrays(1, &meshes[QUAD].VAO);
  glGenBuffers(1, &meshes[QUAD].VBO);
  glGenBuffers(1, &meshes[QUAD].EBO);
  glBindVertexArray(meshes[QUAD].VAO);
  glBindBuffer(GL_ARRAY_BUFFER, meshes[QUAD].VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[QUAD].EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vert), cube_vert, GL_STATIC_DRAW);
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

  return true;
}

void __render_destroy() {
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

void render_viewport(size_t w, size_t h) {
  glViewport(0, 0, w, h);
}

void render_viewport(size_t w0, size_t h0, size_t w, size_t h) {
  glViewport(w0, h0, w, h);
}

void render_clear(color4 color, clear flag) {
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

void render_clear(color3 color, clear flag) {
  render_clear(color4{color, 1.0f}, flag);
}
void render_stencil_test(bool flag) {
  if (flag) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
}

void render_depth_test(bool flag) {
  if (flag) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void render_blending(bool flag) {
  if (flag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO: render_blend_fun
  } else {
    glDisable(GL_BLEND);
  }
}

void render_depth_fun(depth_fun fun) {
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

void render_draw_cube() {
  glBindVertexArray(meshes[CUBE].VAO);
  glDrawArrays(GL_TRIANGLES, 0, meshes[CUBE].draw_count);
  glBindVertexArray(0);
}

void render_draw_quad() {
  glBindVertexArray(meshes[QUAD].VAO);
  glDrawElements(GL_TRIANGLES, meshes[QUAD].draw_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void render_draw_text(const font& font, vec2 pos, float scale, std::string_view text) {
  glBindVertexArray(meshes[TEXT].VAO);

  float x = pos.x, y = pos.y;
  std::string_view::const_iterator c;
  for (c = text.begin(); c != text.end(); ++c) {
    auto [tex, ch] = font._chara.at(*c);

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

} // namespace ntf::shogle
