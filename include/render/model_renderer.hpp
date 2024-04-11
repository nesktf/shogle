#pragma once

#include "res/model.hpp"

namespace ntf {

class ModelRenderer {
protected:
  using res_t = ModelRes;

  ModelRenderer(res_t* mod, Shader* sha) :
    model(mod),
    _shader(sha) {}

public:
  void draw(void);

protected:
  res_t* model;
  Shader* _shader;
};

} // namespace ntf
