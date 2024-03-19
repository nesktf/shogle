#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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
  void unif_int(const char* name, int value) const;
  void unif_float(const char* name, float value) const;
  void unif_vec2(const char* name, const glm::vec2& vec) const;
  void unif_vec3(const char* name, const glm::vec3& vec) const;
  void unif_vec4(const char* name, const glm::vec4& vec) const;
  void unif_mat4(const char* name, const glm::mat4& mat) const;

private:
  void use(void) const;

private:
  GLuint prog;
};

} // namespace ntf::shogle::res

