#pragma once

#include <shogle/core/types.hpp>
#include <shogle/fs/res/shader.hpp>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <memory>

namespace ntf::render {

class Shader {
public:
  using data_t = fs::shader_data;

public: // Resources can't be copied
  Shader(const Shader::data_t* data);
  ~Shader();

  Shader(Shader&&) noexcept;
  Shader(const Shader&) = delete;
  Shader& operator=(Shader&&) noexcept;
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

} // namespace ntf::render
