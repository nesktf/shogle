#pragma once

#include "res/model.hpp"

namespace ntf {

class ModelRenderer {
protected:
  using res_t = ModelRes;

  ModelRenderer(res_t* mod, Shader* sha) :
    _model(mod),
    _shader(sha) {}

public:
  void draw_model(void);

protected:
  res_t* _model;
  Shader* _shader;
};

} // namespace ntf
