#pragma once

#include <shogle/render/texture.hpp>
#include <shogle/res/texture.hpp>

#include <unordered_map>
#include <vector>
#include <queue>

namespace ntf::shogle {

struct sprite {
  const texture2d* texture;
  vec4 texture_offset;
  vec2 sprite_size;
};
using sprite_sequence = std::vector<uint>;


class sprite_group {
public:
  sprite_group(const texture2d* tex, strmap<sprite_sequence> seq, std::vector<vec4> off, vec2 sz);

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
  spritesheet(texture2d_data tex, strmap<sprite_group> sprites, tex_filter filter, tex_wrap wrap);

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
  ~spritesheet() = default;
  spritesheet(spritesheet&&) noexcept;
  spritesheet(const spritesheet&) = delete;
  spritesheet& operator=(spritesheet&&) noexcept;
  spritesheet& operator=(const spritesheet&) = delete;

private:
  void update_groups_texture();

private:
  texture2d _texture;
  strmap<sprite_group> _sprite_groups;
};


class sprite_animator {
public:
  sprite_animator() = default;
  sprite_animator(const sprite_group& sprite_group, std::string_view first_sequence);

public:
  void enqueue_sequence(std::string_view sequence, uint loops);
  void enqueue_sequence_frames(std::string_view sequence, uint frames);
  void soft_switch(std::string_view sequence, uint loops);
  void hard_switch(std::string_view sequence, uint loops);

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
  void reset_queue(bool hard);
  
private:
  const sprite_group* _sprite_group;
  std::queue<entry> _sequences;
};


struct spritesheet_data {
public:
  spritesheet_data(std::string_view path);

public:
  texture2d_data texture;
  strmap<sprite_group> sprite_groups;
};


inline spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = spritesheet_data{path};
  return spritesheet{std::move(data.texture), std::move(data.sprite_groups), filter, wrap};
}

} // namespace ntf::shogle
