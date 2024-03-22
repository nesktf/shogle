#include "resource/shader.hpp"

#include "log.hpp"

#include <fstream>
#include <sstream>

namespace ntf::shogle::res {

// Shader::data_t
std::string _load_shader_file(std::string path) { std::string out;
  std::fstream fs{path};
  if (!fs.is_open()) {
    Log::fatal("[ShaderData] File not found: {}", path);
  } else {
    std::ostringstream ss;
    ss << fs.rdbuf();
    out = ss.str();
  }
  fs.close();
  return out;
}

ShaderData::ShaderData(std::string path) :
  vert_src(_load_shader_file(path+".vs.glsl")),
  frag_src(_load_shader_file(path+".fs.glsl")) {
  Log::verbose("[ShaderData] Shader data extracted (path: {})", path);
}

// Shader
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

Shader::Shader(Shader&& sh) :
  prog(std::move(sh.prog)) {
  sh.prog = 0;
}

Shader& Shader::operator=(Shader&& sh) {
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

} // namespace ntf::shogle::res

