#pragma once

#include <shogle/render/model.hpp>
#include <shogle/render/texture.hpp>

namespace ntf::render {

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

public:
  bool alt_draw {false};

protected:
  const res_t* _res;
  const Shader* _shader;
};

using SpriteRenderer = ObjRenderer<Texture>;
template<>
void SpriteRenderer::draw(void);

using ModelRenderer = ObjRenderer<Model>;
template<>
void ModelRenderer::draw(void);

} // namespace ntf::render
