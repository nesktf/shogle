#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

class model {
public:
  using renderer = gl;
  using loader_t = res::model_loader;

public:
  model(loader_t loader);

  ~model() = default;
  model(model&&) = default;
  model(const model&) = delete;
  model& operator=(model&&) = default;
  model& operator=(const model&) = delete;

public:
  void draw(shader& shader) const;

private:
  std::vector<renderer::mesh> _meshes;
};

} // namespace ntf::render
