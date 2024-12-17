#pragma once

#include "../render.hpp"

#include <glad/glad.h>

#define glCheckError() ::ntf::gl_check_error(__FILE__, __LINE__)

namespace ntf {

class gl_context;

GLenum gl_attrib_type_underlying_cast(r_attrib_type type);
GLenum gl_primitive_cast(r_primitive primitive);

GLenum gl_check_error(const char* file, int line);

} // namespace ntf
