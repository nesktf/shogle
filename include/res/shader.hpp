#pragma once

#include "core/types.hpp"

#include "glad/glad.h"

#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <memory>

namespace ntf {

// Shader::data_t
class ShaderData {
public: // Resource data can be copied but i don't think is a good idea
  ShaderData(std::string path);
  ~ShaderData() = default;

  ShaderData(ShaderData&&) = default;
  ShaderData& operator=(ShaderData&&) = default;

  ShaderData(const ShaderData&) = delete;
  ShaderData& operator=(const ShaderData&) = delete;

public:
  std::string vert_src;
  std::string frag_src;
};

// Shader
class Shader {
public:
  using data_t = ShaderData;

public: // Resources can't be copied
  Shader(const Shader::data_t* data);
  ~Shader();

  Shader(Shader&&) noexcept;
  Shader& operator=(Shader&&) noexcept;

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

public: // Shader uniform manipulation
  inline void use(void) const {
    glUseProgram(this->prog);
  }
  inline void unif_int(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(this->prog, name), value);
  }
  inline void unif_float(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(this->prog, name), value);
  }
  inline void unif_vec2(const char* name, const vec2& vec) const {
    glUniform2fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_vec3(const char* name, const vec3& vec) const {
    glUniform3fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_vec4(const char* name, const vec4& vec) const {
    glUniform4fv(glGetUniformLocation(this->prog, name), 1, glm::value_ptr(vec));
  }
  inline void unif_mat4(const char* name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(this->prog, name), 1, GL_FALSE, glm::value_ptr(mat));
  }

private:
  GLuint prog;
};

} // namespace ntf
