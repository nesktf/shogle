#define SHOGLE_ASSETS_ATLAS_INL
#include <shogle/assets/atlas.hpp>
#undef SHOGLE_ASSETS_ATLAS_INL

namespace ntf {

template<typename Texture>
texture_atlas<Texture>::data_type::data_type(std::string_view path, tex_filter filter, tex_wrap wrap) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};
  json json_data = json::parse(f);

  auto texture_path = file_dir(path.data())+"/"+json_data["file"].get<std::string>();
  texture = texture_data_type{texture_path, filter, wrap};

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


template<typename Texture>
void texture_animator<Texture>::reset_queue(bool hard) {
  if (_sequences.size() == 0) {
    return;
  }

  auto front = std::move(_sequences.front());
  _sequences = std::queue<entry>{};
  if (!hard) {
    _sequences.push(std::move(front));
  }
}

template<typename Texture>
void texture_animator<Texture>::enqueue_sequence(std::string_view sequence, uint loops) {
  const auto& seq = _atlas->sequence_at(sequence);
  _sequences.push(entry{.sequence = seq, .duration = loops*static_cast<uint>(seq.size())});
}

template<typename Texture>
void texture_animator<Texture>::enqueue_sequence_frames(std::string_view sequence, uint frames) {
  enqueue_sequence(sequence, 0);
  auto& enqueued = _sequences.back();
  enqueued.duration = frames;
}

template<typename Texture>
void texture_animator<Texture>::soft_switch(std::string_view sequence, uint loops) {
  reset_queue(false);
  enqueue_sequence(sequence, loops);
}

template<typename Texture>
void texture_animator<Texture>::hard_switch(std::string_view sequence, uint loops) {
  reset_queue(true);
  enqueue_sequence(sequence, loops);
}

template<typename Texture>
void texture_animator<Texture>::tick() {
  auto& next = _sequences.front();
  const auto& seq = next.sequence;
  uint& clock = next.clock;
  clock++;
  if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
    _sequences.pop();
  }
}

template<typename Texture>
auto texture_animator<Texture>::next_frame() const -> texture_atlas<texture_type>::texture {
  const auto& next = _sequences.front();
  return next.sequence[next.clock%next.sequence.size()];
}

} // namespace ntf
