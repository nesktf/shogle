#pragma once

#include "core/game_object.hpp"
#include "core/model.hpp"

namespace ntf::shogle {

class ModelObject : public GameObject {
public:
  ModelObject(const Model& model, Shader& shader);
  ~ModelObject() = default;

  ModelObject(ModelObject&&) = default;
  ModelObject& operator=(ModelObject&&) = default;

  ModelObject(const ModelObject&) = delete;
  ModelObject& operator=(const ModelObject&) = delete;

public:
  virtual void update(float dt) override;
  void draw(void) const override;

public:
  std::reference_wrapper<const Model> model;
};

} // namespace ntf::shogle
