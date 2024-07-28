#pragma once

#include <shogle/render/texture.hpp>
#include <shogle/res/texture.hpp>

#include <unordered_map>
#include <vector>

namespace ntf::shogle {

struct sprite_data {
  uint delay;
  vec2 scale;
  vec2 linear_offset;
  std::vector<vec2> const_offset;
};


struct spritesheet_data {
public:
  spritesheet_data(std::string_view path_);

public:
  texture2d_data texture;
  strmap<sprite_data> sprites;
};


class sprite {
public:
  sprite() = default;
  sprite(const texture2d* tex, const std::vector<vec2>* coff, vec2 loff, vec2 scale, uint delay);

public:
  const texture2d& tex() const { return *_tex; }
  const vec2& scale() const { return _scale; }
  size_t count() const { return _const_offset->size(); }
  uint delay() const { return _delay; }

  const vec4& offset() const { return _offset; }
  vec4 offset_at(size_t i) const { return vec4{_linear_offset, *(_const_offset->begin()+i)}; }

public:
  void set_index(size_t i) { _offset = offset_at(i); }

private:
  const texture2d* _tex;
  const std::vector<vec2>* _const_offset;
  vec2 _linear_offset;
  vec4 _offset;
  vec2 _scale;
  uint _delay;
};


enum class sprite_animation : uint8_t {
  none = 0,
  repeat = 1 << 0,
  forward_anim = 1 << 1,
};

constexpr sprite_animation operator|(sprite_animation a, sprite_animation b) {
  return static_cast<sprite_animation>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr sprite_animation operator&(sprite_animation a, sprite_animation b) {
  return static_cast<sprite_animation>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr sprite_animation& operator|=(sprite_animation& a, sprite_animation b) {
  return a = static_cast<sprite_animation>(a | b);
}


class sprite_animator {
public:
  sprite_animator() = default;
  sprite_animator(uint delay_, sprite_animation animation_);

public:
  void tick(sprite& sprite);
  void set_index(sprite& sprite, uint index);

public:
  uint delay{1};
  sprite_animation animation{sprite_animation::forward_anim | sprite_animation::repeat};

private:
  uint _index_time{0}, _index{0};
};


class spritesheet {
public:
  spritesheet() = default;
  spritesheet(texture2d_data tex_data, strmap<sprite_data> sprites, tex_filter filter, tex_wrap wrap);

public:
  sprite at(std::string_view name) const;
  sprite operator[](std::string_view name) const { return at(name); };

  const texture2d& tex() const { return _texture; }
  size_t size() const { return _sprites.size(); }

private:
  texture2d _texture;
  strmap<sprite_data> _sprites;
};


inline spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = spritesheet_data{path};
  return spritesheet{std::move(data.texture), std::move(data.sprites), filter, wrap};
}

} // namespace ntf::shogle
