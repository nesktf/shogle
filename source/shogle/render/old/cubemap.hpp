#pragma once

namespace ntf::render {

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
