#pragma once

#include <shogle/render/texture.hpp>

#include <shogle/res/texture.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <nlohmann/json.hpp>

#include <unordered_map>
#include <vector>
#include <queue>

namespace ntf {

struct sprite {
  const texture2d* texture;
  vec4 texture_offset;
  vec2 sprite_size;
};
using sprite_sequence = std::vector<uint>;


class sprite_group {
public:
  inline sprite_group(const texture2d* tex, strmap<sprite_sequence> seq, std::vector<vec4> off, vec2 sz);

public:
  sprite sprite_at(uint index) const { return sprite {
    .texture = _texture,
    .texture_offset = _offsets[index%_offsets.size()],
    .sprite_size = _sprite_size
  };}

  sprite sprite_at(std::string_view sequence, uint frame) const {
    const auto& seq = sequence_at(sequence);
    return sprite_at(seq[frame%seq.size()]);
  }

  const sprite_sequence& sequence_at(std::string_view name) const { return _sequences.at(name.data()); }

  const texture2d& tex() const { return *_texture; }

private:
  const texture2d* _texture;
  strmap<sprite_sequence> _sequences;
  std::vector<vec4> _offsets;
  vec2 _sprite_size;

private:
  friend class spritesheet;
};


class spritesheet {
public:
  spritesheet() = default;
  inline spritesheet(texture2d_data tex, strmap<sprite_group> sprites, tex_filter filter, tex_wrap wrap);

public:
  sprite sprite_at(std::string_view group_name, uint index) const {
    const auto& group = _sprite_groups.at(group_name.data());
    return group.sprite_at(index);
  }

  sprite sprite_at(std::string_view group_name, std::string_view sequence_name, uint frame) const {
    const auto& seq = sequence_at(group_name, sequence_name);
    return sprite_at(group_name, seq[frame%seq.size()]);
  }

  const sprite_sequence& sequence_at(std::string_view group_name, std::string_view sequence_name) const {
    const auto& group = _sprite_groups.at(group_name.data());
    return group.sequence_at(sequence_name);
  }

  const sprite_group& group_at(std::string_view name) const {
    return _sprite_groups.at(name.data());
  }

  const texture2d& tex() const { return _texture; }
  size_t size() const { return _sprite_groups.size(); }

public:
  inline ~spritesheet() = default;
  inline spritesheet(spritesheet&&) noexcept;
  spritesheet(const spritesheet&) = delete;
  inline spritesheet& operator=(spritesheet&&) noexcept;
  spritesheet& operator=(const spritesheet&) = delete;

private:
  inline void update_groups_texture();

private:
  texture2d _texture;
  strmap<sprite_group> _sprite_groups;
};


class sprite_animator {
public:
  sprite_animator() = default;
  inline sprite_animator(const sprite_group& sprite_group, std::string_view first_sequence);

public:
  inline void enqueue_sequence(std::string_view sequence, uint loops);
  inline void enqueue_sequence_frames(std::string_view sequence, uint frames);
  inline void soft_switch(std::string_view sequence, uint loops);
  inline void hard_switch(std::string_view sequence, uint loops);

public:
  void tick() {
    auto& next = _sequences.front();
    const auto& seq = next.sequence;
    uint& clock = next.clock;
    clock++;
    if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
      _sequences.pop();
    }
  }

  sprite frame() const { 
    const auto& next = _sequences.front();
    const uint frame = next.sequence[next.clock];
    return _sprite_group->sprite_at(frame);
  }

private:
  struct entry {
    const sprite_sequence& sequence;
    uint duration, clock{0};
  };

private:
  inline void reset_queue(bool hard);
  
private:
  const sprite_group* _sprite_group;
  std::queue<entry> _sequences;
};


namespace impl {

using json = nlohmann::json;

inline sprite_sequence parse_sequence(uint delay, const json& json_sequence) {
  uint total_frames = json_sequence.size()*delay;
  sprite_sequence sequence(total_frames);

  for (uint i = 0, curr_frame = 0; i < total_frames; i += delay) {
    uint index = json_sequence[curr_frame];
    for (uint j = 0; j < delay; ++j) {
      sequence[j+i] = index;
    }
    curr_frame++;
  }

  return sequence;
}

inline std::vector<vec4> parse_offsets(ivec2 tex_sz, vec2& sprite_sz, const json& offset_entry) {
  size_t count = offset_entry["count"].get<size_t>();
  std::vector<vec4> offsets(count);

  size_t x = static_cast<size_t>(tex_sz.x);
  size_t y = static_cast<size_t>(tex_sz.y);

  size_t x0 = offset_entry["x0"].get<size_t>();
  size_t y0 = offset_entry["y0"].get<size_t>();
  size_t dx = offset_entry["dx"].get<size_t>();
  size_t dy = offset_entry["dy"].get<size_t>();

  size_t cols = offset_entry["cols"].get<size_t>();
  float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
  size_t rows = static_cast<size_t>(std::ceil(row_ratio));

  sprite_sz = vec2{(float)(dx*rows) / (float)(dy*cols), 1.0f};

  vec2 linear_offset {(float)dx / (float)(x*cols), (float)dy / (float)(y*rows)};

  for (size_t j = 0; j < count; ++j) {
    size_t row = j / cols;
    size_t col = j % cols;

    vec2 frac_a {(x0*cols) + (col*dx), y0 + (row*dy)};
    vec2 frac_b {x*cols, y*rows};
    offsets[j] = vec4{linear_offset, vec2{frac_a.x/frac_b.x, frac_a.y/frac_b.y}};
  }

  return offsets;
}

} // namespace impl

struct spritesheet_data {
public:
  inline spritesheet_data(std::string_view path);

public:
  texture2d_data texture;
  strmap<sprite_group> sprite_groups;
};


inline spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = spritesheet_data{path};
  return spritesheet{std::move(data.texture), std::move(data.sprite_groups), filter, wrap};
}

sprite_group::sprite_group(const texture2d* tex, strmap<sprite_sequence> seq, std::vector<vec4> off, vec2 sz) :
  _texture(tex), _sequences(std::move(seq)), _offsets(std::move(off)), _sprite_size(sz) {}


spritesheet::spritesheet(texture2d_data tex, strmap<sprite_group> sprites, tex_filter filter, tex_wrap wrap) :
  _texture(load_texture(tex.pixels, tex.width, tex.height, tex.format, filter, wrap)),
  _sprite_groups(std::move(sprites)) { update_groups_texture(); }

void spritesheet::update_groups_texture() {
  for (auto& [name, group] : _sprite_groups) {
    group._texture = &_texture;
  }
}

spritesheet::spritesheet(spritesheet&& s) noexcept :
  _texture(std::move(s._texture)), _sprite_groups(std::move(s._sprite_groups)) { update_groups_texture(); }

spritesheet& spritesheet::operator=(spritesheet&& s) noexcept {
  _texture = std::move(s._texture);
  _sprite_groups = std::move(s._sprite_groups);

  update_groups_texture();

  return *this;
}

sprite_animator::sprite_animator(const sprite_group& sprite_group, std::string_view first_sequence) :
  _sprite_group(&sprite_group) { enqueue_sequence(first_sequence, 0); }

void sprite_animator::reset_queue(bool hard) {
  if (_sequences.size() == 0) {
    return;
  }

  auto front = std::move(_sequences.front());
  _sequences = std::queue<entry>{};
  if (!hard) {
    _sequences.push(std::move(front));
  }
}

void sprite_animator::enqueue_sequence(std::string_view sequence, uint loops) {
  const auto& seq = _sprite_group->sequence_at(sequence);
  _sequences.push(entry{ .sequence = seq, .duration = loops*static_cast<uint>(seq.size()) });
}

void sprite_animator::enqueue_sequence_frames(std::string_view sequence, uint frames) {
  enqueue_sequence(sequence, 0);
  auto& enqueued = _sequences.back();
  enqueued.duration = frames;
}

void sprite_animator::soft_switch(std::string_view sequence, uint loops) {
  reset_queue(false);
  enqueue_sequence(sequence, loops);
}

void sprite_animator::hard_switch(std::string_view sequence, uint loops) {
  reset_queue(true);
  enqueue_sequence(sequence, loops);
}

spritesheet_data::spritesheet_data(std::string_view path) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};
  json json_data = json::parse(f);

  auto texture_path = file_dir(path.data())+"/"+json_data["file"].get<std::string>();
  texture = texture2d_data{texture_path};

  ivec2 tex_size { texture.width, texture.height };
  for (const auto& content : json_data["content"]) {
    vec2 sprite_size;
    auto group_name = content["name"].get<std::string>();
    auto group_offsets = impl::parse_offsets(tex_size, sprite_size, content["offset"]);

    strmap<sprite_sequence> group_sequences;
    for (const auto& anim : content["anim"]) {
      auto seq_name = anim["name"].get<std::string>();
      uint delay = anim["delay"].get<uint>();

      const auto& sequence = anim["sequence"];
      group_sequences.emplace(std::make_pair(std::move(seq_name), impl::parse_sequence(delay, sequence)));
    }

    sprite_groups.emplace(std::make_pair(std::move(group_name),
      sprite_group{nullptr, std::move(group_sequences), std::move(group_offsets), sprite_size}));
  }
}

} // namespace ntf
