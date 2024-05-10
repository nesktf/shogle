#include <shogle/render/shader.hpp>

namespace ntf::render {

shader::shader(const char* vert_src, const char* frag_src) :
  _shader(vert_src, frag_src) {}

shader::shader(std::string path) :
  shader(loader_t{std::move(path)}) {}

shader::shader(loader_t loader) :
  _shader(loader) {}

shader::~shader() {
  gl::destroy_shader(_shader);
}

} // namespace ntf::render
