#pragma once

#include <shogle/res/texture.hpp>

#include <nlohmann/json.hpp>

#include <unordered_map>
#include <vector>
#include <queue>

namespace ntf {

class spritesheet {
public:
  using sprite = uint32_t;

  using group = std::vector<sprite>;
  using sequence = std::vector<sprite>;

  struct sprite_data {
    vec4 offset;
    vec2 base_size;
  };

public:
  spritesheet() = default;
  spritesheet(texture2d_data tex, std::vector<sprite_data> sprites, 
              strmap<group> groups, strmap<sequence> seq) :
    _texture(impl::load_texture<1u>(tex.pixels, tex.dim, tex.format, tex.filter, tex.wrap)),
    _sprites(std::move(sprites)), _sprite_groups(std::move(groups)), _sprite_sequences(std::move(seq)) {}

public:
  const sequence& sequence_at(std::string_view name) const { return _sprite_sequences.at(name.data()); }
  const group& group_at(std::string_view name) const { return _sprite_groups.at(name.data()); }
  const sprite_data& at(sprite id) const { return _sprites.at(id); }
  const sprite_data& operator[](sprite id) const { return _sprites[id]; }
  const texture2d& tex() const { return _texture; }
  size_t size() const { return _sprites.size(); }
  bool valid() const { return _texture.valid() && _sprites.size() > 0; }

private:
  texture2d _texture;
  std::vector<sprite_data> _sprites;
  strmap<group> _sprite_groups;
  strmap<sequence> _sprite_sequences;
};

class sprite_animator {
private:
  struct entry {
    const spritesheet::sequence& sequence;
    uint duration, clock{0};
  };

public:
  sprite_animator() = default;
  sprite_animator(const spritesheet& sheet, std::string_view first_sequence) :
    _spritesheet(&sheet) { enqueue_sequence(first_sequence, 0); }

public:
  void enqueue_sequence(std::string_view sequence, uint loops);
  void enqueue_sequence_frames(std::string_view sequence, uint frames);
  void soft_switch(std::string_view sequence, uint loops);
  void hard_switch(std::string_view sequence, uint loops);

public:
  void tick();
  spritesheet::sprite next_frame() const;

private:
  void reset_queue(bool hard);
  
private:
  const spritesheet* _spritesheet;
  std::queue<entry> _sequences;
};

struct spritesheet_data {
public:
  struct loader {
    spritesheet operator()(spritesheet_data data) {
      return spritesheet{std::move(data.texture), std::move(data.sprites), 
                         std::move(data.groups), std::move(data.sequences)};
    }
  };

public:
  spritesheet_data(std::string_view path, tex_filter filter, tex_wrap wrap);

public:
  texture2d_data texture;
  std::vector<spritesheet::sprite_data> sprites;
  strmap<spritesheet::sequence> sequences;
  strmap<spritesheet::group> groups;
};


inline void sprite_animator::reset_queue(bool hard) {
  if (_sequences.size() == 0) {
    return;
  }

  auto front = std::move(_sequences.front());
  _sequences = std::queue<entry>{};
  if (!hard) {
    _sequences.push(std::move(front));
  }
}

inline void sprite_animator::enqueue_sequence(std::string_view sequence, uint loops) {
  const auto& seq = _spritesheet->sequence_at(sequence);
  _sequences.push(entry{.sequence = seq, .duration = loops*static_cast<uint>(seq.size())});
}

inline void sprite_animator::enqueue_sequence_frames(std::string_view sequence, uint frames) {
  enqueue_sequence(sequence, 0);
  auto& enqueued = _sequences.back();
  enqueued.duration = frames;
}

inline void sprite_animator::soft_switch(std::string_view sequence, uint loops) {
  reset_queue(false);
  enqueue_sequence(sequence, loops);
}

inline void sprite_animator::hard_switch(std::string_view sequence, uint loops) {
  reset_queue(true);
  enqueue_sequence(sequence, loops);
}

inline void sprite_animator::tick() {
  auto& next = _sequences.front();
  const auto& seq = next.sequence;
  uint& clock = next.clock;
  clock++;
  if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
    _sequences.pop();
  }
}

inline spritesheet::sprite sprite_animator::next_frame() const {
  const auto& next = _sequences.front();
  return next.sequence[next.clock%next.sequence.size()];
}

inline spritesheet_data::spritesheet_data(std::string_view path, tex_filter filter, tex_wrap wrap) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};
  json json_data = json::parse(f);

  auto texture_path = file_dir(path.data())+"/"+json_data["file"].get<std::string>();
  texture = texture2d_data{texture_path, filter, wrap};

  auto parse_sprites = [this](const json& entry) -> std::vector<spritesheet::sprite_data> {
    size_t count = entry["count"].get<size_t>();
    std::vector<spritesheet::sprite_data> data(count);

    size_t x = static_cast<size_t>(texture.dim.x);
    size_t y = static_cast<size_t>(texture.dim.y);

    size_t x0 = entry["x0"].get<size_t>();
    size_t y0 = entry["y0"].get<size_t>();
    size_t dx = entry["dx"].get<size_t>();
    size_t dy = entry["dy"].get<size_t>();

    size_t cols = entry["cols"].get<size_t>();
    float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
    size_t rows = static_cast<size_t>(std::ceil(row_ratio));

    vec2 base_size {(float)(dx*rows) / (float)(dy*cols), 1.0f};

    vec2 linear_offset {(float)dx / (float)(x*cols), (float)dy / (float)(y*rows)};

    for (size_t j = 0; j < count; ++j) {
      size_t row = j / cols;
      size_t col = j % cols;

      vec2 frac_a {(x0*cols) + (col*dx), y0 + (row*dy)};
      vec2 frac_b {x*cols, y*rows};
      vec4 offset{linear_offset, vec2{frac_a.x/frac_b.x, frac_a.y/frac_b.y}};

      data[j] = spritesheet::sprite_data{.offset = offset, .base_size = base_size};
    }

    return data;
  };

  auto parse_sequences = [](size_t off, std::string gname, const json& entry) -> strmap<spritesheet::sequence> {
    strmap<spritesheet::sequence> sequences;

    for (const auto& seq_entry : entry) {
      auto& sequence = seq_entry["sequence"];
      std::string seq_name = seq_entry["name"].get<std::string>();
      uint delay = seq_entry["delay"].get<uint>();

      uint total_frames = sequence.size()*delay;
      spritesheet::sequence new_sequence(total_frames);
      for (uint i = 0, curr_frame = 0; i < total_frames; i += delay) {
        uint index = sequence[curr_frame];
        for (uint j = 0; j < delay; ++j) {
          new_sequence[j+i] = off+index;
        }
        curr_frame++;
      }

      // Names the sequence like "marisa.idle" for group "marisa"
      sequences.emplace(std::make_pair(gname+"."+seq_name, std::move(new_sequence)));
    }

    return sequences;
  };

  size_t sprite_count = 0;
  for (const auto& content : json_data["content"]) {
    std::string group_name = content["name"].get<std::string>();

    std::vector<spritesheet::sprite_data> group_sprites = parse_sprites(content["offset"]);
    strmap<spritesheet::sequence> group_sequences = parse_sequences(sprite_count, group_name, content["anim"]);

    spritesheet::group group_ids(group_sprites.size());
    for (size_t i = 0; i < group_sprites.size(); ++i) {
      group_ids[i] = i+sprite_count;
    }

    sprite_count += group_sprites.size();

    sprites.insert(sprites.end(), group_sprites.begin(), group_sprites.end());
    sequences.merge(std::move(group_sequences)); // Makes all sequences public for all groups but who cares
    groups.emplace(std::make_pair(std::move(group_name), std::move(group_ids)));
  }
}


inline spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  spritesheet_data::loader loader;
  return loader(spritesheet_data{path, filter, wrap});
}

} // namespace ntf
