#include <shogle/render/gl.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#define DEFAULT_WRAP GL_REPEAT

namespace ntf::render {

struct common_vaos : public Singleton<common_vaos> {
  struct vao {
    GLuint vao, vbo, ebo;
  };

  common_vaos() = default;

  void init(void);
  void destroy(void);

  vao quad2d, quad2d_inv;
  vao quad3d, quad3d_inv;
  vao cube;
};

void gl::init(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    throw ntf::error("[gl] Failed to load GLAD");
  }
  common_vaos::instance().init();
  Log::debug("[gl] Initialized");
}

void gl::destroy(void) {
  common_vaos::instance().destroy();
  Log::debug("[gl] Terminated");
}

void gl::set_tex_filter(texture& texture, res::texture_filter filter) {
  switch (filter) {
    case res::texture_filter::nearest: {
      texture.filter = GL_NEAREST;
      break;
    };
    case res::texture_filter::linear: {
      texture.filter = GL_LINEAR;
      break;
    }
  };
  glBindTexture(texture.type, texture.id);
  glTexParameteri(texture.type, GL_TEXTURE_MIN_FILTER, texture.filter);
  glTexParameteri(texture.type, GL_TEXTURE_MAG_FILTER, texture.filter);
  glBindTexture(texture.type, 0);
  Log::verbose("[gl::texture] Texture filter modified (id: {}, filter: {})", texture.id, texture.filter);
}

void gl::draw_mesh(const mesh& mesh) {
  glBindVertexArray(mesh.vao);
  glDrawElements(GL_TRIANGLES, mesh.indices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void gl::draw_quad_2d(bool inverted) {
  auto& vaos {common_vaos::instance()};
  glBindVertexArray(inverted ? vaos.quad2d_inv.vao : vaos.quad2d.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void gl::draw_quad_3d(bool inverted) {
  auto& vaos {common_vaos::instance()};
  glBindVertexArray(inverted ? vaos.quad3d_inv.vao : vaos.quad3d.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void gl::draw_cube(void) {
  auto& vaos {common_vaos::instance()};
  glBindVertexArray(vaos.cube.vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

// texture
gl::texture::texture(size_t w, size_t h, res::texture_format tex_format, res::texture_filter tex_filter) {
  width = w;
  height = h;

  type = GL_TEXTURE_2D;

  switch (tex_format) {
    case res::texture_format::mono: {
      format = GL_RED;
      break;
    }
    case res::texture_format::rgba: {
      format = GL_RGBA;
      break;
    }
    case res::texture_format::rgb: {
      format = GL_RGB;
      break;
    }
  }
  
  switch (tex_filter) {
    case res::texture_filter::nearest: {
      filter = GL_NEAREST;
      break;
    }
    case res::texture_filter::linear: {
      filter = GL_LINEAR;
      break;
    }
  }

  glGenTextures(1, &id);
  glBindTexture(type, id);
  glTexImage2D(type, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filter);
  glBindTexture(type, 0);
  Log::verbose("[gl::texture] Empty texture created (id: {}, type: {})", id, type);
}

gl::texture::texture(loader_t loader) {
  width = loader.width;
  height = loader.height;

  switch (loader.type) {
    case res::texture_type::tex2d: {
      type = GL_TEXTURE_2D;
      break;
    }
    case res::texture_type::cubemap: {
      type = GL_TEXTURE_CUBE_MAP;
      break;
    }
  }

  switch (loader.format) {
    case res::texture_format::mono: {
      format = GL_RED;
      break;
    }
    case res::texture_format::rgba: {
      format = GL_RGBA;
      break;
    }
    case res::texture_format::rgb: {
      format = GL_RGB;
      break;
    }
  }
  
  switch (loader.filter) {
    case res::texture_filter::nearest: {
      filter = GL_NEAREST;
      break;
    }
    case res::texture_filter::linear: {
      filter = GL_LINEAR;
      break;
    }
  }

  glGenTextures(1, &id);
  glBindTexture(type, id);
  glTexImage2D(type, 0, format, loader.width, loader.height, 0, format, GL_UNSIGNED_BYTE, loader.pixels);
  glGenerateMipmap(type);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(type, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glBindTexture(type, 0);
  Log::verbose("[gl::texture] Texture created (id: {}, type: {})", id, type);
}

void gl::destroy_texture(texture& tex) {
  if (!tex.id) return;

  Log::verbose("[gl::texture] Texture deleted (id: {}, type: {})", tex.id, tex.type);
  glDeleteTextures(1, &tex.id);

  tex.id = 0;
}

// framebuffer
gl::framebuffer::framebuffer(size_t w, size_t h) :
  tex(w, h, res::texture_format::rgb, res::texture_filter::nearest) {

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.type, tex.id, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error("[GL Renderer] Incomplete framebuffer");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  Log::verbose("[gl::framebuffer] Framebuffer created (id: {})", fbo);
}

void gl::destroy_framebuffer(framebuffer& fbo) {
  if (!fbo.fbo) return;

  Log::verbose("[gl::framebuffer] Framebuffer deleted (id: {})", fbo.fbo);
  glDeleteFramebuffers(1, &fbo.fbo);
  glDeleteBuffers(1, &fbo.rbo); // ?

  fbo.fbo = 0;
}

// mesh
gl::mesh::mesh(loader_t loader) {
  using vertex = loader_t::vertex;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, loader.vertices.size()*sizeof(vertex), &loader.vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, loader.indices.size()*sizeof(uint), &loader.indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coord));

  glBindVertexArray(0);

  indices = loader.indices.size();

  for (auto& mat : loader.materials) {
    materials.emplace_back(
      texture{std::move(mat.texture)},
      std::move(mat.uniform_name)
    );
  }
  Log::verbose("[gl::mesh] Mesh created (id: {}, material_count: {})", vao, materials.size());
}

void gl::destroy_mesh(mesh& mesh) {
  if (!mesh.vao) return;

  for (auto& mat : mesh.materials) {
    gl::destroy_texture(mat.tex);
  }
  mesh.materials.clear();

  Log::verbose("[gl::mesh] Mesh deleted (id: {})", mesh.vao);
  glBindVertexArray(mesh.vao);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &mesh.vao);
  glDeleteBuffers(1, &mesh.ebo);
  glDeleteBuffers(1, &mesh.vbo);

  mesh.vao = 0;
}

// shader
gl::shader::shader(loader_t loader) {
  int succ;
  char log[512];

  const char* vert_src = loader.vert_src.c_str();
  const char* frag_src = loader.frag_src.c_str();

  auto vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vert_src, nullptr);
  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    glDeleteShader(vert); // ?
    throw ntf::error{"[gl::shader] Vertex shader compilation falied: {}", log};
  }

  auto frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &frag_src, nullptr);
  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(frag, 512, nullptr, log);
    glDeleteShader(vert);
    glDeleteShader(frag); // ??
    throw ntf::error{"[gl::shader] Fragment shader compilation falied: {}", log};
  }

  prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);
  glGetProgramiv(prog, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    std::string err {fmt::format("[Shader] Shader linking failed: {}", log)};
    glDeleteShader(frag);
    glDeleteShader(vert);
    throw ntf::error{"[gl::shader] Shader linking failed: {}", log};
  }
  glDeleteShader(frag);
  glDeleteShader(vert);
  Log::verbose("[gl::shader] Shader created (id: {})", prog);
}

void gl::destroy_shader(shader& shad) {
  if (!shad.prog) return;

  Log::verbose("[gl::shader] Shader deleted (id: {})", shad.prog);
  glDeleteProgram(shad.prog);

  shad.prog = 0;
}

void common_vaos::init() {
  // inverted texture quads are considered "normal" for convenience
  float quad2d_vert[] = { // inverted in texture space (for stb_image textures)
    // coord        // tex_coord
    -0.5f, -0.5f,   0.0f, 0.0f,
     0.5f, -0.5f,   1.0f, 0.0f,
     0.5f,  0.5f,   1.0f, 1.0f,
    -0.5f,  0.5f,   0.0f, 1.0f
  };
  float quad2d_vert_inv[] = { // not inverted (for framebuffers)
    // coord        // tex_coord
    -0.5f, -0.5f,   0.0f, 1.0f,
     0.5f, -0.5f,   1.0f, 1.0f,
     0.5f,  0.5f,   1.0f, 0.0f,
    -0.5f,  0.5f,   0.0f, 0.0f
  };

  float quad3d_vert[] = { // inverted in texture space (for stb_image textures)
    // coord                // normal           // tex_coord
    -0.5f, -0.5f,  0.0f,     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
     0.5f, -0.5f,  0.0f,     0.0f,  0.0f,  1.0f,   1.0f,  0.0f,
     0.5f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
    -0.5f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,   0.0f,  1.0f
  };
  float quad3d_vert_inv[] = { // not inverted (for framebuffers)
    // coord                // normal           // tex_coord
    -0.5f, -0.5f,  0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
     0.5f, -0.5f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    -0.5f,  0.5f,  0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f
  };
  GLuint quad_ind[] = {
    0, 1, 2, // bottom right triangle
    0, 2, 3  // top left triangle
  };

  float cube_vert[] = {
    // coord                 // normal             // tex_coord
    -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  1.0f
  };

  { // quad2d
    glGenVertexArrays(1, &quad2d.vao);
    glGenBuffers(1, &quad2d.vbo);
    glGenBuffers(1, &quad2d.ebo);

    glBindVertexArray(quad2d.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad2d.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad2d_vert), quad2d_vert, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad2d.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

    // Sprite shader needs a vec4f with 2d coords and tex coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glBindVertexArray(0);
  }

  { // quad2d inverted
    glGenVertexArrays(1, &quad2d_inv.vao);
    glGenBuffers(1, &quad2d_inv.vbo);
    glGenBuffers(1, &quad2d_inv.ebo);

    glBindVertexArray(quad2d_inv.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad2d_inv.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad2d_vert_inv), quad2d_vert_inv, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad2d_inv.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

    // Sprite shader needs a vec4f with 2d coords and tex coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

    glBindVertexArray(0);
  }

  { // quad3d
    glGenVertexArrays(1, &quad3d.vao);
    glGenBuffers(1, &quad3d.vbo);
    glGenBuffers(1, &quad3d.ebo);

    glBindVertexArray(quad3d.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad3d.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad3d_vert), quad3d_vert, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad3d.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // coords
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(2); // tex_coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

    glBindVertexArray(0);
  }

  { // quad3d inverted
    glGenVertexArrays(1, &quad3d_inv.vao);
    glGenBuffers(1, &quad3d_inv.vbo);
    glGenBuffers(1, &quad3d_inv.ebo);

    glBindVertexArray(quad3d_inv.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad3d_inv.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad3d_vert_inv), quad3d_vert_inv, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad3d_inv.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // coords
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(2); // tex_coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

    glBindVertexArray(0);
  }

  { // cube
    glGenVertexArrays(1, &cube.vao);
    glGenBuffers(1, &cube.vbo);
    // glGenBuffers(1, &cube.ebo);

    glBindVertexArray(cube.vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vert), cube_vert, GL_STATIC_DRAW);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // coords
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(2); // tex_coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

    glBindVertexArray(0);
  }


  Log::verbose("[gl::common_vaos] VAOs initialized");
}

void common_vaos::destroy(void) {

  { // cube
    glBindVertexArray(cube.vao);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &cube.vao);
    // glDeleteBuffers(1, &cube.ebo);
    glDeleteBuffers(1, &cube.vbo);
  }

  { // quad3d inverted
    glBindVertexArray(quad3d_inv.vao);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quad3d_inv.vao);
    glDeleteBuffers(1, &quad3d_inv.ebo);
    glDeleteBuffers(1, &quad3d_inv.vbo);
  }

  { // quad3d
    glBindVertexArray(quad3d_inv.vao);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quad3d.vao);
    glDeleteBuffers(1, &quad3d.ebo);
    glDeleteBuffers(1, &quad3d.vbo);
  }

  { // quad2d inverted
    glBindVertexArray(quad2d_inv.vao);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quad2d_inv.vao);
    glDeleteBuffers(1, &quad2d_inv.ebo);
    glDeleteBuffers(1, &quad2d_inv.vbo);
  }

  { // quad2d
    glBindVertexArray(quad2d.vao);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quad2d.vao);
    glDeleteBuffers(1, &quad2d.ebo);
    glDeleteBuffers(1, &quad2d.vbo);
  }

  Log::verbose("[gl::common_vaos] VAOs deleted");
}


} // namespace ntf::render
