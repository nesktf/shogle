#pragma once

#include "resources/shader_data.hpp"
#include "glad/glad.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace ntf::shogle {

class Shader {
public:
  Shader(const ShaderData* data);
  ~Shader();

  void set_int(const char* name, int value);
  void set_float(const char* name, float value);
  void set_vec2(const char* name, const glm::vec2& vec);
  void set_vec3(const char* name, const glm::vec3& vec);
  void set_vec4(const char* name, const glm::vec4& vec);
  void set_mat4(const char* name, const glm::mat4& mat);

private:
  void use(void);

  GLuint vert, frag, prog;
};

}
