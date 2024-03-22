#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

#include <string>
#include <memory>

namespace ntf::shogle::res {

// Shader::data_t
class ShaderData {
public:
  ShaderData(std::string path);
  ~ShaderData() = default;

public:
  std::string vert_src;
  std::string frag_src;
};

// Shader
class Shader {
public:
  using data_t = ShaderData;

public:
  Shader(const Shader::data_t* data);
  ~Shader();

  Shader(Shader&&);
  Shader& operator=(Shader&&);

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

public:
  inline void use(void) const {
    glUseProgram(this->prog);
  }
  inline void unif_int(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(this->prog, name), value);
  }
  inline void unif_float(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(this->prog, name), value);
  }
  inline void unif_vec2(const char* name, const glm::vec2& vec) const {
    glUniform2fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_vec3(const char* name, const glm::vec3& vec) const {
    glUniform3fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_vec4(const char* name, const glm::vec4& vec) const {
    glUniform4fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_mat4(const char* name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(this->prog, name), 1, GL_FALSE, glm::value_ptr(mat));
  }

private:
  GLuint prog;
};

} // namespace ntf::shogle::res

