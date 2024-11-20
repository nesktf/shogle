#define SHOGLE_ASSETS_ATLAS_INL
#include "./atlas.hpp"
#undef SHOGLE_ASSETS_ATLAS_INL

namespace ntf {

template<typename Texture>
texture_atlas<Texture>::data_type::data_type(std::string_view path, tex_filter filter,
                                             tex_wrap wrap) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};
  json json_data = json::parse(f);

  auto texture_path = file_dir(path.data())+"/"+json_data["file"].get<std::string>();
  texture = texture_data_type{texture_path, filter, wrap};

  auto parse_meta = [this](const json& json_entry) {
    size_t count = json_entry["count"].get<size_t>();
    std::vector<texture_atlas::texture_meta> data(count);

    size_t x = static_cast<size_t>(texture.dim.x);
    size_t y = static_cast<size_t>(texture.dim.y);

    size_t x0 = json_entry["x0"].get<size_t>();
    size_t y0 = json_entry["y0"].get<size_t>();
    size_t dx = json_entry["dx"].get<size_t>();
    size_t dy = json_entry["dy"].get<size_t>();

    size_t cols = json_entry["cols"].get<size_t>();
    float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
    size_t rows = static_cast<size_t>(std::ceil(row_ratio));

    ivec2 dim {dx/cols, dy/rows}; // all the sprites in the group have the same size

    vec2 linear_offset = (vec2)dim/(vec2)texture.dim;

    for (size_t j = 0; j < count; ++j) {
      size_t row = j / cols;
      size_t col = j % cols;

      vec2 frac_a {(x0*cols) + (col*dx), (y0*rows) + (row*dy)};
      vec2 frac_b {x*cols, y*rows};
      vec4 offset{linear_offset, vec2{frac_a.x/frac_b.x, frac_a.y/frac_b.y}};

      data[j] = texture_atlas::texture_meta{.offset = offset, .dim = dim};
    }

    return data;
  };

  size_t total_sprite_count = 0;
  auto parse_sequences = [&](const std::string& group_name, const json& json_entry) {
    std::vector<std::pair<std::string, texture_atlas::texture_vec>> sequences(json_entry.size());

    size_t seq_pos = 0;
    for (const auto& seq_entry : json_entry) {
      auto& sequence = seq_entry["sequence"];
      std::string seq_name = seq_entry["name"].get<std::string>();
      uint delay = seq_entry["delay"].get<uint>();
      uint total_frames = sequence.size()*delay;

      texture_atlas::texture_vec new_sequence(total_frames);
      for (uint i = 0, curr_frame = 0; i < total_frames; i += delay) {
        uint index = sequence[curr_frame];
        for (uint j = 0; j < delay; ++j) {
          new_sequence[j+i] = total_sprite_count+index;
        }
        curr_frame++;
      }

      // Names the sequence like "marisa.idle" for group "marisa"
      sequences[seq_pos++] = std::make_pair(group_name+"."+seq_name, std::move(new_sequence));
    }

    return sequences;
  };

  auto create_group = [&](std::string group_name, size_t group_size) {
    std::pair<std::string, texture_atlas::texture_vec> group_data(
      std::make_pair(std::move(group_name), texture_atlas::texture_vec(group_size)));

    for (size_t i = 0; i < group_size; ++i) {
      group_data.second[i] = i+total_sprite_count;
    }
    total_sprite_count += group_size;

    return group_data;
  };

  for (const auto& content : json_data["content"]) {
    std::string group_name = content["name"].get<std::string>();

    auto group_metas = parse_meta(content["offset"]); // group's sprites sizes & offsets
    auto group_sequences = parse_sequences(group_name, content["anim"]); // group's sequences
    auto group = create_group(std::move(group_name), group_metas.size()); // group name & sprite ids

    metas.insert(metas.end(), 
                 std::make_move_iterator(group_metas.begin()),
                 std::make_move_iterator(group_metas.end()));
    sequences.insert(sequences.end(),
                     std::make_move_iterator(group_sequences.begin()),
                     std::make_move_iterator(group_sequences.end()));
    groups.emplace_back(std::move(group));
  }
}

template<typename Texture>
auto texture_atlas<Texture>::data_type::loader::operator()(data_type data) -> texture_atlas {
  typename texture_type::loader tex_loader;
  auto& tdata = data.texture;
  auto tex = tex_loader(tdata.pixels, tdata.dim, tdata.format, tdata.filter, tdata.wrap);
  return texture_atlas{std::move(tex), std::move(data.metas), 
                       std::move(data.groups), std::move(data.sequences)};
}

template<typename Texture>
auto texture_atlas<Texture>::data_type::loader::operator()(std::string path, tex_filter filter, 
                                                           tex_wrap wrap) -> texture_atlas {
  return (*this)(data_type{path, filter, wrap});
}

template<typename Texture>
texture_atlas<Texture>::texture_atlas(texture_type texture, std::vector<texture_meta> metas,
                                      std::vector<std::pair<std::string, texture_vec>> groups,
                                      std::vector<std::pair<std::string, texture_vec>> sequences) :
  _texture(std::move(texture)), _metas(std::move(metas)), _groups(groups.size()),
  _sequences(sequences.size()) {
  for (atlas_group i = 0; i < groups.size(); ++i) {
    auto& [name, group] = groups[i];
    _groups[i] = std::move(group);
    _group_names.emplace(std::make_pair(std::move(name), i));
  }
  for (atlas_sequence i = 0; i < sequences.size(); ++i) {
    auto& [name, sequence] = sequences[i];
    _sequences[i] = std::move(sequence);
    _sequence_names.emplace(std::make_pair(std::move(name), i));
  }
}

template<typename Texture>
auto texture_atlas<Texture>::at(atlas_texture tex) const -> const texture_meta& {
  // return _metas.at(tex);
  assert(tex < _metas.size() && "Texture id out of range");
  return _metas[tex];
}

template<typename Texture>
auto texture_atlas<Texture>::operator[](atlas_texture tex) const -> const texture_meta& {
  assert(tex < _metas.size() && "Texture id out of range");
  return _metas[tex];
}

template<typename Texture>
auto texture_atlas<Texture>::sequence_at(atlas_sequence seq) const -> const texture_vec& {
  assert(seq < _sequences.size() && "Sequence id out of range");
  return _sequences[seq];
}

template<typename Texture>
auto texture_atlas<Texture>::group_at(atlas_group group) const -> const texture_vec& {
  assert(group < _groups.size() && "Group id out of range");
  return _groups[group];
}

template<typename Texture>
auto texture_atlas<Texture>::find_sequence(std::string_view name) const
                                                                -> std::optional<atlas_sequence> {
  auto it = _sequence_names.find(name.data());
  if (it != _sequence_names.end()) {
    return {static_cast<atlas_sequence>(it->second)};
  }

  return {};
}

template<typename Texture>
auto texture_atlas<Texture>::find_group(std::string_view name) const 
                                                                -> std::optional<atlas_sequence> {
  auto it = _group_names.find(name.data());
  if (it != _group_names.end()) {
    return {static_cast<atlas_group>(it->second)};
  }

  return {};
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::reset_queue(bool hard) {
  if (_sequences.size() == 0) {
    return;
  }

  auto front = std::move(_sequences.front());
  _sequences = std::queue<entry>{};
  if (!hard) {
    _sequences.push(std::move(front));
  }
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::enqueue_sequence(sequence_handle sequence, uint loops) {
  const auto& seq = _atlas->sequence_at(sequence);
  _sequences.push(entry{
    .sequence=sequence,
    .duration = loops*static_cast<uint>(seq.size())
  });
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::enqueue_sequence_frames(sequence_handle sequence,
                                                                  uint frames) {
  enqueue_sequence(sequence, 0);
  auto& enqueued = _sequences.back();
  enqueued.duration = frames;
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::soft_switch(sequence_handle sequence, uint loops) {
  reset_queue(false);
  enqueue_sequence(sequence, loops);
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::hard_switch(sequence_handle sequence, uint loops) {
  reset_queue(true);
  enqueue_sequence(sequence, loops);
}

template<typename Texture, typename AtlasPtr>
void texture_animator<Texture, AtlasPtr>::tick() {
  auto& next = _sequences.front();
  const auto& seq = _atlas->sequence_at(next.sequence);
  uint& clock = next.clock;
  clock++;
  if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
    _sequences.pop();
  }
}

template<typename Texture, typename AtlasPtr>
auto texture_animator<Texture, AtlasPtr>::frame() const -> atlas_texture {
  const auto& next = _sequences.front();
  const auto& seq = _atlas->sequence_at(next.sequence);
  return seq[next.clock%seq.size()];
}

} // namespace ntf
