#pragma once

#include "res/model.hpp"
#include "res/texture.hpp"

namespace ntf {

template<typename T>
class ObjRenderer {
public:
  using res_t = T;

protected:
  ObjRenderer(const T* res, const Shader* shader) :
    _res(res),
    _shader(shader) {}

public:
  void draw(void) {};

protected:
  const res_t* _res;
  const Shader* _shader;
};

using SpriteRenderer = ObjRenderer<Texture>;
template<>
void SpriteRenderer::draw(void);

using ModelRenderer = ObjRenderer<ModelRes>;
template<>
void ModelRenderer::draw(void);

} // namespace ntf
