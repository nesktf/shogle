#pragma once

#include <shogle/res/texture.hpp>

#include <nlohmann/json.hpp>

#include <unordered_map>
#include <vector>
#include <queue>

namespace ntf {

class texture_atlas {
public:
  using texture = uint32_t;

  struct texture_meta {
  public:
    vec2 aspect() const { return vec2{(float)dim.x/(float)dim.y, 1.f};}

  public:
    vec4 offset;
    ivec2 dim;
  };

  using group = std::vector<texture>;
  using sequence = std::vector<texture>;

public:
  texture_atlas() = default;

  texture_atlas(texture2d tex) : _texture(std::move(tex)), 
    _metas{default_meta(_texture.dim())} {} // Just wrap a texture

  texture_atlas(texture2d tex, std::vector<texture_meta> sprites, 
                strmap<group> groups = {}, strmap<sequence> seq = {}) :
    _texture(std::move(tex)), _metas(std::move(sprites)),
    _groups(std::move(groups)), _sequences(std::move(seq)) {}

public:
  const sequence& sequence_at(std::string_view name) const { return _sequences.at(name.data()); }
  const group& group_at(std::string_view name) const { return _groups.at(name.data()); }

  const texture_meta& at(texture id) const { return _metas.at(id); }
  const texture_meta& operator[](texture id) const { return _metas[id]; }

  const texture2d& tex() const { return _texture; }
  texture2d& tex() { return _texture; }

  size_t size() const { return _metas.size(); }
  bool valid() const { return _texture.valid() && _metas.size() > 0; }
  bool has(texture id) const { return _metas.size() > id; }

  operator const texture2d&() const { return _texture; }

public:
  texture_meta default_meta(ivec2 dim) { return texture_meta{.offset=vec4{vec2{1.f},vec2{0.f}},.dim=dim}; }

private:
  texture2d _texture;
  std::vector<texture_meta> _metas;
  strmap<group> _groups;
  strmap<sequence> _sequences;
};

class texture_animator {
private:
  struct entry {
    const texture_atlas::sequence& sequence;
    uint duration, clock{0};
  };

public:
  texture_animator() = default;
  texture_animator(const texture_atlas& atlas, std::string_view first_sequence) :
    _atlas(&atlas) { enqueue_sequence(first_sequence, 0); }

public:
  void enqueue_sequence(std::string_view sequence, uint loops);
  void enqueue_sequence_frames(std::string_view sequence, uint frames);
  void soft_switch(std::string_view sequence, uint loops);
  void hard_switch(std::string_view sequence, uint loops);

public:
  void tick();
  texture_atlas::texture next_frame() const;

private:
  void reset_queue(bool hard);
  
private:
  const texture_atlas* _atlas;
  std::queue<entry> _sequences;
};

struct texture_atlas_data {
public:
  struct loader {
    texture_atlas operator()(texture_atlas_data data) {
      auto& tdata = data.texture;
      texture2d tex{impl::load_texture<1u>(tdata.pixels, tdata.dim, tdata.format, tdata.filter, tdata.wrap)};
      return texture_atlas{std::move(tex), std::move(data.sprites), 
        std::move(data.groups), std::move(data.sequences)};
    }
  };

public:
  texture_atlas_data(std::string_view path, tex_filter filter, tex_wrap wrap);

public:
  texture2d_data texture;
  std::vector<texture_atlas::texture_meta> sprites;
  strmap<texture_atlas::sequence> sequences;
  strmap<texture_atlas::group> groups;
};


inline void texture_animator::reset_queue(bool hard) {
  if (_sequences.size() == 0) {
    return;
  }

  auto front = std::move(_sequences.front());
  _sequences = std::queue<entry>{};
  if (!hard) {
    _sequences.push(std::move(front));
  }
}

inline void texture_animator::enqueue_sequence(std::string_view sequence, uint loops) {
  const auto& seq = _atlas->sequence_at(sequence);
  _sequences.push(entry{.sequence = seq, .duration = loops*static_cast<uint>(seq.size())});
}

inline void texture_animator::enqueue_sequence_frames(std::string_view sequence, uint frames) {
  enqueue_sequence(sequence, 0);
  auto& enqueued = _sequences.back();
  enqueued.duration = frames;
}

inline void texture_animator::soft_switch(std::string_view sequence, uint loops) {
  reset_queue(false);
  enqueue_sequence(sequence, loops);
}

inline void texture_animator::hard_switch(std::string_view sequence, uint loops) {
  reset_queue(true);
  enqueue_sequence(sequence, loops);
}

inline void texture_animator::tick() {
  auto& next = _sequences.front();
  const auto& seq = next.sequence;
  uint& clock = next.clock;
  clock++;
  if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
    _sequences.pop();
  }
}

inline texture_atlas::texture texture_animator::next_frame() const {
  const auto& next = _sequences.front();
  return next.sequence[next.clock%next.sequence.size()];
}

inline texture_atlas_data::texture_atlas_data(std::string_view path, tex_filter filter, tex_wrap wrap) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};
  json json_data = json::parse(f);

  auto texture_path = file_dir(path.data())+"/"+json_data["file"].get<std::string>();
  texture = texture2d_data{texture_path, filter, wrap};

  auto parse_meta = [this](const json& entry) -> std::vector<texture_atlas::texture_meta> {
    size_t count = entry["count"].get<size_t>();
    std::vector<texture_atlas::texture_meta> data(count);

    size_t x = static_cast<size_t>(texture.dim.x);
    size_t y = static_cast<size_t>(texture.dim.y);

    size_t x0 = entry["x0"].get<size_t>();
    size_t y0 = entry["y0"].get<size_t>();
    size_t dx = entry["dx"].get<size_t>();
    size_t dy = entry["dy"].get<size_t>();

    size_t cols = entry["cols"].get<size_t>();
    float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
    size_t rows = static_cast<size_t>(std::ceil(row_ratio));

    ivec2 dim {dx/cols, dy/rows}; // all sprites in the group are of the same size

    vec2 linear_offset = (vec2)dim/(vec2)texture.dim;
    // vec2 linear_offset {(float)dx / (float)(x*cols), (float)dy / (float)(y*rows)};

    for (size_t j = 0; j < count; ++j) {
      size_t row = j / cols;
      size_t col = j % cols;

      vec2 frac_a {(x0*cols) + (col*dx), y0 + (row*dy)};
      vec2 frac_b {x*cols, y*rows};
      vec4 offset{linear_offset, vec2{frac_a.x/frac_b.x, frac_a.y/frac_b.y}};

      data[j] = texture_atlas::texture_meta{.offset = offset, .dim = dim};
    }

    return data;
  };

  auto parse_seq = [](size_t off, std::string gname, const json& entry) -> strmap<texture_atlas::sequence> {
    strmap<texture_atlas::sequence> sequences;

    for (const auto& seq_entry : entry) {
      auto& sequence = seq_entry["sequence"];
      std::string seq_name = seq_entry["name"].get<std::string>();
      uint delay = seq_entry["delay"].get<uint>();

      uint total_frames = sequence.size()*delay;
      texture_atlas::sequence new_sequence(total_frames);
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

  size_t sprite_count = 1;
  for (const auto& content : json_data["content"]) {
    std::string group_name = content["name"].get<std::string>();

    std::vector<texture_atlas::texture_meta> group_metas = parse_meta(content["offset"]);
    strmap<texture_atlas::sequence> group_sequences = parse_seq(sprite_count, group_name, content["anim"]);

    texture_atlas::group group_ids(group_metas.size());
    for (size_t i = 1; i <= group_metas.size(); ++i) {
      group_ids[i-1] = i+sprite_count;
    }

    sprite_count += group_metas.size();

    sprites.insert(sprites.end(), group_metas.begin(), group_metas.end());
    sequences.merge(std::move(group_sequences)); // Makes all sequences public for all groups but who cares
    groups.emplace(std::make_pair(std::move(group_name), std::move(group_ids)));
  }
}


inline texture_atlas load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  texture_atlas_data::loader loader;
  return loader(texture_atlas_data{path, filter, wrap});
}

} // namespace ntf
