#pragma once

#include "resource/texture.hpp"
#include "resource/shader.hpp"

#include "traits.hpp"

namespace ntf::shogle::render {

class Sprite {
public:
  Sprite(cref<res::Texture> _texture, cref<res::Shader> _shader) :
    texture_ref(_texture),
    shader_ref(_shader) {}

public:
  void draw(void);
  static glm::mat4 model_transform(TransformData transform);

public:
  cref<res::Texture> texture_ref;
  cref<res::Shader> shader_ref;
  glm::mat4 model_m {1.0f};
};

} // namespace ntf::shogle::render

namespace ntf::shogle {

template<typename T>
requires(is_drawable<T>)
class GameObject;

using SpriteObj = GameObject<render::Sprite>;

} // namespace ntf::shogle
