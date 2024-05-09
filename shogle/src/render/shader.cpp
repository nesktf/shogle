#include <shogle/render/shader.hpp>

namespace ntf::render {

shader::shader(std::string path) :
  shader(loader_t{std::move(path)}) {}

shader::shader(loader_t loader) :
  _shader(loader) {}

shader::~shader() {
  gl::destroy_shader(_shader);
}

} // namespace ntf::render
