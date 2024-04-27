#include <shogle/render/backends/gl.hpp>

#include <shogle/core/error.hpp>

#define DEFAULT_WRAP GL_REPEAT

namespace ntf::render {

struct gl_quad_2d : public Singleton<gl_quad_2d> {
  gl_quad_2d();
  ~gl_quad_2d();
  GLuint vao, vbo, ebo;
  GLuint vao_inv, vbo_inv, ebo_inv;
};

void gl::init(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    throw ntf::error("[GL Renderer] Failed to load GLAD");
  }
}

void gl::set_tex_filter(texture& texture, res::texture_filter filter) {
  switch (filter) {
    case res::texture_filter::nearest: {
      texture.glfilter = GL_NEAREST;
      break;
    };
    case res::texture_filter::linear: {
      texture.glfilter = GL_LINEAR;
      break;
    }
  };
  texture.filter = filter;
  glBindTexture(texture.gltype, texture.id);
  glTexParameteri(texture.gltype, GL_TEXTURE_MIN_FILTER, texture.glfilter);
  glTexParameteri(texture.gltype, GL_TEXTURE_MAG_FILTER, texture.glfilter);
  glBindTexture(texture.gltype, 0);
}

void gl::draw_mesh(const mesh& mesh) {
  glBindVertexArray(mesh.vao);
  glDrawElements(GL_TRIANGLES, mesh.indices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void gl::draw_quad(const texture& texture, bool inverted) {
  auto& quad {gl_quad_2d::instance()};
  glBindTexture(texture.gltype, texture.id);
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(inverted ? quad.vao_inv : quad.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// texture
gl::texture::texture(size_t w, size_t h, res::texture_format format, res::texture_filter filter) {
  gltype = GL_TEXTURE_2D;
  type = res::texture_type::tex2d;
  switch (format) {
    case res::texture_format::mono: {
      glformat = GL_RED;
      break;
    }
    case res::texture_format::rgba: {
      glformat = GL_RGBA;
      break;
    }
    case res::texture_format::rgb: {
      glformat = GL_RGB;
      break;
    }
  }
  
  switch (filter) {
    case res::texture_filter::nearest: {
      glfilter = GL_NEAREST;
      break;
    }
    case res::texture_filter::linear: {
      glfilter = GL_LINEAR;
      break;
    }
  }

  glGenTextures(1, &id);
  glBindTexture(gltype, id);
  glTexImage2D(gltype, 0, glformat, w, h, 0, glformat, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltype, 0);

}
gl::texture::texture(res::texture_loader loader) {
  switch (loader.type) {
    case res::texture_type::tex2d: {
      gltype = GL_TEXTURE_2D;
      break;
    }
    case res::texture_type::cubemap: {
      gltype = GL_TEXTURE_CUBE_MAP;
      break;
    }
  }

  switch (loader.format) {
    case res::texture_format::mono: {
      glformat = GL_RED;
      break;
    }
    case res::texture_format::rgba: {
      glformat = GL_RGBA;
      break;
    }
    case res::texture_format::rgb: {
      glformat = GL_RGB;
      break;
    }
  }
  
  switch (loader.filter) {
    case res::texture_filter::nearest: {
      glfilter = GL_NEAREST;
      break;
    }
    case res::texture_filter::linear: {
      glfilter = GL_LINEAR;
      break;
    }
  }

  glGenTextures(1, &id);
  glBindTexture(gltype, id);
  glTexImage2D(gltype, 0, glformat, loader.width, loader.height, 0, glformat, GL_UNSIGNED_BYTE, loader.pixels);
  glGenerateMipmap(gltype);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glBindTexture(gltype, 0);
}

gl::texture::~texture() {
  if (!id) return;
  glDeleteTextures(1, &id);
}

gl::texture::texture(texture&& t) noexcept :
  id(t.id), glformat(t.glformat), gltype(t.gltype), glfilter(t.glfilter),
  format(t.format), type(t.type), filter(t.filter),
  width(t.width), height(t.height) { t.id = 0; }

gl::texture& gl::texture::operator=(texture&& t) noexcept {
  id = t.id; glformat = t.glformat; gltype = t.gltype; glfilter = t.glfilter;
  format = t.format; type = t.type; filter = t.filter;
  width = t.width; height = t.height;

  t.id = 0;

  return *this;
}

// framebuffer
gl::framebuffer::framebuffer(size_t w, size_t h) :
  texture(w, h, res::texture_format::rgb, res::texture_filter::nearest) {
  auto dim = texture.gltype;

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dim, texture.id, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error("[GL Renderer] Incomplete framebuffer");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

gl::framebuffer::~framebuffer() {
  if (!fbo) return;
  glDeleteFramebuffers(1, &fbo);
  glDeleteBuffers(1, &rbo); // ?
}

gl::framebuffer::framebuffer(framebuffer&& f) noexcept :
  fbo(f.fbo), rbo(f.rbo), texture(std::move(f.texture)) { f.fbo = 0; f.rbo = 0; }

gl::framebuffer& gl::framebuffer::operator=(framebuffer&& f) noexcept {
  fbo = f.fbo; rbo = f.rbo; texture = std::move(f.texture);

  f.fbo = 0; f.rbo = 0;

  return *this;
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
}

gl::mesh::~mesh() {
  if (!vao) return;
  glBindVertexArray(vao);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
}

gl::mesh::mesh(mesh&& m) noexcept :
  vao(m.vao), vbo(m.vbo), ebo(m.ebo),
  indices(m.indices), materials(std::move(m.materials)) { m.vao = 0; m.vbo = 0; m.ebo = 0; }

gl::mesh& gl::mesh::operator=(mesh&& m) noexcept {
  vao = m.vao; vbo = m.vbo; ebo = m.ebo;
  indices = m.indices; materials = std::move(m.materials);

  m.vao = 0; m.vbo = 0; m.ebo = 0;

  return *this;
}

// shader
gl::shader::shader(res::shader_loader loader) {
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
    throw ntf::error{"[Shader] Vertex shader compilation falied: {}", log};
  }

  auto frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &frag_src, nullptr);
  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(frag, 512, nullptr, log);
    glDeleteShader(vert);
    glDeleteShader(frag); // ??
    throw ntf::error{"[Shader] Fragment shader compilation falied: {}", log};
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
    throw ntf::error{"[Shader] Shader linking failed: {}", log};
  }
  glDeleteShader(frag);
  glDeleteShader(vert);
}

gl::shader::~shader() {
  if (!prog) return;
  glDeleteProgram(prog);
}

gl::shader::shader(shader&& s) noexcept :
  prog(s.prog) { s.prog = 0; }

gl::shader& gl::shader::operator=(shader&& s) noexcept {
  prog = s.prog;

  s.prog = 0;

  return *this;
}

gl_quad_2d::gl_quad_2d() {
  float quad_vert[] = {
    // coord      // tex_coord
    -0.5f, -0.5f, 0.0f, 0.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
     0.5f,  0.5f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 1.0f
  };
  float quad_vert_inv[] = { // for framebuffers
    // coord      // tex_coord
    -0.5f, -0.5f, 0.0f, 1.0f,
     0.5f, -0.5f, 1.0f, 1.0f,
     0.5f,  0.5f, 1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f
  };
  GLuint quad_ind[] = {
    0, 1, 2,
    0, 2, 3
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Sprite shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &vao_inv);
  glGenBuffers(1, &vbo_inv);
  glGenBuffers(1, &ebo_inv);

  glBindVertexArray(vao_inv);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_inv);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert_inv), quad_vert_inv, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_inv);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);
}

gl_quad_2d::~gl_quad_2d() {
  glBindVertexArray(vao_inv);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao_inv);
  glDeleteBuffers(1, &ebo_inv);
  glDeleteBuffers(1, &vbo_inv);

  glBindVertexArray(vao);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
}


} // namespace ntf::render
