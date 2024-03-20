#include "resource/shader.hpp"

#include "log.hpp"

#include <glm/gtc/type_ptr.hpp>

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

void Shader::unif_int(const char* name, int value) const {
  this->use();
  glUniform1i(glGetUniformLocation(this->prog, name), value);
}

void Shader::unif_float(const char* name, float value) const {
  this->use();
  glUniform1f(glGetUniformLocation(this->prog, name), value);
}

void Shader::unif_vec2(const char* name, const glm::vec2& vec) const {
  this->use();
  glUniform2fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::unif_vec3(const char* name, const glm::vec3& vec) const {
  this->use();
  glUniform3fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::unif_vec4(const char* name, const glm::vec4& vec) const {
  this->use();
  glUniform4fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::unif_mat4(const char* name, const glm::mat4& mat) const {
  this->use();
  glUniformMatrix4fv(glGetUniformLocation(this->prog, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::use(void) const {
  glUseProgram(this->prog);
}

} // namespace ntf::shogle::res

