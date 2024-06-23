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


class texture2d {
public:
  texture2d(uint8_t* data, size_t w, size_t h, tex_format format, tex_filter filter, tex_wrap wrap);

public:
  texture2d& set_filter(tex_filter filter);
  texture2d& set_wrap(tex_wrap wrap);

public:
  vec2sz dim() const { return _dim; }
  GLuint id() const { return _id; }

public:
  ~texture2d();
  texture2d(texture2d&&) noexcept;
  texture2d(const texture2d&) = delete;
  texture2d& operator=(texture2d&&) noexcept;
  texture2d& operator=(const texture2d&) = delete;

private:
  vec2sz _dim;
  GLint _format, _filter, _wrap;
  GLuint _id;

private:
  friend void render_bind_sampler(const texture2d& texture, size_t sampler);
};

class cubemap {
public:
  cubemap(std::array<uint8_t*, CUBEMAP_FACES> data, size_t dim, tex_format format, tex_filter filter, tex_wrap wrap);

public:
  cubemap& set_filter(tex_filter filter);
  cubemap& set_wrap(tex_wrap wrap);

public:
  vec2sz dim() const { return _dim; }

public:
  ~cubemap();
  cubemap(cubemap&&) noexcept;
  cubemap(const cubemap&) = delete;
  cubemap& operator=(cubemap&&) noexcept;
  cubemap& operator=(const cubemap&) = delete;

private:
  vec2sz _dim;
  GLint _format, _filter, _wrap;
  GLuint _id;

private:
  friend void render_bind_sampler(const cubemap& cubemap, size_t sampler);
};


void render_bind_sampler(const texture2d& tex, size_t sampler);
void render_bind_sampler(const cubemap& cubemap, size_t sampler);

} // namespace ntf::shogle
