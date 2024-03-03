#include "core/sprite.hpp"
#include "core/renderer.hpp"

namespace ntf::shogle {

void Sprite::draw(Shader& shader) {
  Renderer::instance().draw(shader, *this);
}


}
