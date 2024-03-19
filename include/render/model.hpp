#pragma once

#include "render/drawable.hpp"
#include "resource/model.hpp"

namespace ntf::shogle {

class Model : public Drawable {
public:
  Model(res::Model* _model, res::Shader* _shader) :
    Drawable(_shader),
    model(_model) {}

public:
  virtual void draw(void) override;

public:
  res::Model* model;
};

} // namespace ntf::shogle
