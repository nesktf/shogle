#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

struct model {
public:
  using loader_t = res::model_loader;

public:
  model(std::string path);
  model(loader_t loader);

public:
  std::vector<gl::mesh> _meshes;

public:
  ~model();
  model(model&&) = default;
  model(const model&) = delete;
  model& operator=(model&&) noexcept;
  model& operator=(const model&) = delete;
};

void draw_model(model& model, shader& shader);

} // namespace ntf::render
