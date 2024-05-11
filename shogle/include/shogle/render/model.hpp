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

struct cubemap {
public:
  using loader_t = res::texture_loader;

public:
  cubemap(std::string path);
  cubemap(loader_t loader);

public:
  gl::texture _tex;

public:
  ~cubemap();
  cubemap(cubemap&&) = default;
  cubemap(const cubemap&) = delete;
  cubemap& operator=(cubemap&&) noexcept;
  cubemap& operator=(const cubemap&) = delete;
};

void draw_cubemap(cubemap& cube, shader& shader);

} // namespace ntf::render
