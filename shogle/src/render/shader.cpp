#include <shogle/render/res/shader.hpp>

#include <shogle/core/log.hpp>

namespace ntf::render {

Shader::Shader(const Shader::data_t* data) {
  int succ;
  char log[512];

  const char* vertx_src = data->vert_src.c_str();
  const char* fragm_src = data->frag_src.c_str();

  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertx_src, nullptr);
  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    Log::error("[Shader] Vertex shader compilation failed: {}", log);
  }

  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragm_src, nullptr);
  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(frag, 512, nullptr, log);
    Log::error("[Shader] Fragment shader compilation failed: {}", log);
  }

  this->prog = glCreateProgram();
  glAttachShader(this->prog, vert);
  glAttachShader(this->prog, frag);
  glLinkProgram(this->prog);
  glGetProgramiv(this->prog, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    Log::error("[Shader] Linking failed: {}", log);
  }

  glDeleteShader(frag);
  glDeleteShader(vert);

  Log::verbose("[Shader] Shader created (sha-id: {})", this->prog);
}

Shader::Shader(Shader&& sh) noexcept :
  prog(std::move(sh.prog)) {

  sh.prog = 0;
}

Shader& Shader::operator=(Shader&& sh) noexcept {
  this->prog = std::move(sh.prog);

  sh.prog = 0;

  return *this;
}

Shader::~Shader() {
  if (this->prog == 0) return;
  GLuint id = this->prog;
  glDeleteProgram(this->prog);
  Log::verbose("[Shader] Shader deleted (sha-id: {})", id);
}

} // namespace ntf::render
