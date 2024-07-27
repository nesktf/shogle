#pragma once

#include <shogle/render/render.hpp>

#include <array>

#define CUBEMAP_FACES 6

namespace ntf::shogle {

enum class tex_format {
  mono = 0,
  rgb,
  rgba,
};

enum class tex_filter {
  nearest = 0,
  linear,
  nearest_mp_nearest,
  nearest_mp_linear,
  linear_mp_linear,
  linear_mp_nearest,
};

enum class tex_wrap {
  repeat = 0,
  mirrored_repeat,
  clamp_edge,
  clamp_border,
};


class framebuffer;

class texture2d {
public:
  texture2d() = default;
  texture2d(GLuint id, size_t w, size_t h);
  texture2d(uint8_t* data, size_t w, size_t h, tex_format format);

public:
  texture2d& set_filter(tex_filter filter);
  texture2d& set_wrap(tex_wrap wrap);

public:
  GLuint& id() { return _id; } // Not const
  ivec2 dim() const { return _dim; }
  bool valid() const { return _id != 0; }

public:
  ~texture2d();
  texture2d(texture2d&&) noexcept;
  texture2d(const texture2d&) = delete;
  texture2d& operator=(texture2d&&) noexcept;
  texture2d& operator=(const texture2d&) = delete;

private:
  void unload_texture();

private:
  GLuint _id{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const texture2d& tex, size_t sampler);
  friend void render_bind_sampler(const framebuffer& fb, size_t sampler);
};

texture2d load_texture(uint8_t* data, size_t w, size_t h, tex_format format, tex_filter filter, tex_wrap wrap);


using cmappixels = std::array<uint8_t*, CUBEMAP_FACES>;
class cubemap {
public:
  cubemap() = default;
  cubemap(GLuint id, size_t dim);
  cubemap(cmappixels data, size_t dim, tex_format format);

public:
  cubemap& set_filter(tex_filter filter);
  cubemap& set_wrap(tex_wrap wrap);

public:
  GLuint& id() { return _id; } // Not const
  ivec2 dim() const { return _dim; }
  bool valid() const { return _id != 0; }

public:
  ~cubemap();
  cubemap(cubemap&&) noexcept;
  cubemap(const cubemap&) = delete;
  cubemap& operator=(cubemap&&) noexcept;
  cubemap& operator=(const cubemap&) = delete;

private:
  void unload_cubemap();

private:
  GLuint _id{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const cubemap& cubemap, size_t sampler);
};

cubemap load_cubemap(cmappixels data, size_t dim, tex_format format, tex_filter filter, tex_wrap wrap);


void render_bind_sampler(const texture2d& tex, size_t sampler);
void render_bind_sampler(const cubemap& cubemap, size_t sampler);

} // namespace ntf::shogle
