#include "core/shader.hpp"
#include "core/logger.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace ntf::shogle {

Shader::Shader(std::unique_ptr<ShaderData> data) {
  int succ;
  char log[512];

  const char* vertx_src = data->vertex_src.c_str();
  const char* fragm_src = data->fragmt_src.c_str();

  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertx_src, nullptr);
  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    logger::error("[Shader] Vertex shader compilation failed: {}", log);
  }

  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragm_src, nullptr);
  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(frag, 512, nullptr, log);
    logger::error("[Shader] Fragment shader compilation failed: {}", log);
  }

  this->prog = glCreateProgram();
  glAttachShader(this->prog, vert);
  glAttachShader(this->prog, frag);
  glLinkProgram(this->prog);
  glGetProgramiv(this->prog, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
    logger::error("[Shader] Linking failed: {}", log);
  }

  glDeleteShader(frag);
  glDeleteShader(vert);

  logger::debug("[Shader] Created shader (id: {})", this->prog);
}

Shader::~Shader() {
  GLuint id = this->prog;
  glDeleteProgram(this->prog);
  logger::debug("[Shader] Deleted shader (id: {})", id);
}

void Shader::set_int(const char* name, int value) {
  this->use();
  glUniform1i(glGetUniformLocation(this->prog, name), value);
}

void Shader::set_float(const char* name, float value) {
  this->use();
  glUniform1f(glGetUniformLocation(this->prog, name), value);
}

void Shader::set_vec2(const char* name, const glm::vec2& vec) {
  this->use();
  glUniform2fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::set_vec3(const char* name, const glm::vec3& vec) {
  this->use();
  glUniform3fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::set_vec4(const char* name, const glm::vec4& vec) {
  this->use();
  glUniform4fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
}

void Shader::set_mat4(const char* name, const glm::mat4& mat) {
  this->use();
  glUniformMatrix4fv(glGetUniformLocation(this->prog, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::use(void) {
  glUseProgram(this->prog);
}

} // namespace ntf::shogle
