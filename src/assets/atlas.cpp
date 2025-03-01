#include "./atlas.hpp"
#include "./filesystem.hpp"

#define RET_ERR(msg, ...) \
  SHOGLE_LOG(error, "[ntf::grid_atlas_loader] " msg __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{asset_error::format({msg}__VA_OPT__(,) __VA_ARGS__)}

using nlohmann::json;

namespace ntf {

asset_expected<void> grid_atlas_loader::parse(const std::string& path) {
  auto dir_path = file_dir(path);
  if (!dir_path) {
    RET_ERR("Invalid path \"{}\"", path);
  }

  std::ifstream f{path};
  if (!f.is_open()) {
    RET_ERR("Failed to open json file \"{}\"", path);
  }

  json json_data = json::parse(f, nullptr, false);
  if (json_data.is_discarded()) {
    RET_ERR("Failed to parse json file \"{}\"", path);
  }

  auto tex_path = fmt::format("{}/{}", *dir_path, json_data["file"].get<std::string>());
  auto tex_info = stb_image_loader::parse_meta(tex_path);
  if (!tex_info) {
    RET_ERR("Failed to open texture file \"{}\"", tex_path);
  }

  // TODO: Allocate the array directly instead of using vectors
  {
    uint32 sprite_count = 0;
    uint32 sequence_count = 0;
    for (const auto& content : json_data["content"]) {
      sprite_count += content["offset"]["count"].get<uint32>();
      sequence_count += content["anim"].size();
    }
    _sprites.reserve(sprite_count);
    _groups.reserve(json_data["content"].size());
    _sequences.reserve(sequence_count);

    if (!_sprites.capacity()) {
      _groups.shrink_to_fit();
      _sequences.shrink_to_fit();
      RET_ERR("Failed to preparse json \"{}\"", path);
    }
  }
  SHOGLE_LOG(verbose,
             "[ntf::grid_atlas_loader] Found {} sprites, {} groups, {} sequences for \"{}\"",
             _sprites.capacity(), _groups.capacity(), _sequences.capacity(), path);

  auto parse_sequences = [&](uint32 sprite_pos, const std::string& group_name, const json& entry) {
    for (const auto& seq_entry : entry) {
      const auto& seq = seq_entry["sequence"];
      const uint32 delay = seq_entry["delay"].get<uint32>();
      const uint32 total_frames = seq.size()*delay;

      size_t index_pos = _indices.size();
      _indices.resize(index_pos+total_frames, 0);
      for (uint32 i = 0, curr_frame = 0; i < total_frames; i += delay, ++curr_frame) {
        uint32 index = seq[curr_frame];
        for (uint32 j = 0; j < delay; ++j) {
          _indices[index_pos+j+i] = sprite_pos+index;
        }
      }

      _sequences.emplace_back(
        fmt::format("{}.{}", group_name, seq_entry["name"].get<std::string>()),
        vec_span{
          .index = static_cast<uint32>(index_pos),
          .count = total_frames,
        }
      );
    }
  };
  auto parse_offsets = [&](uint32 tex_w, uint32 tex_h, const json& entry) {
    const uint32 count = entry["count"].get<uint32>();

    const uint32 x0 = entry["x0"].get<uint32>();
    const uint32 y0 = entry["y0"].get<uint32>();
    const uint32 dx = entry["dx"].get<uint32>();
    const uint32 dy = entry["dy"].get<uint32>();

    const uint32 cols = entry["cols"].get<uint32>();
    const float32 row_ratio = static_cast<float32>(count)/static_cast<float32>(cols);
    const uint32 rows = static_cast<uint32>(std::ceil(row_ratio));

    const uvec2 dim{dx/cols, dy/rows}; // all the sprites in the group have the same size
    vec2 linear_offset = static_cast<vec2>(dim);
    linear_offset.x /= static_cast<float32>(tex_w);
    linear_offset.y /= static_cast<float32>(tex_h);

    for (uint32 i = 0; i < count; ++i) {
      const uint32 row = i / cols;
      const uint32 col = i % cols;

      const vec2 frac_a {(x0*cols) + (col*dx), (y0*rows) + (row*dy)};
      const vec2 frac_b {tex_w*cols, tex_h*rows};
      const vec4 offset{linear_offset.x, linear_offset.y, frac_a.x/frac_b.x, frac_a.y/frac_b.y};

      _sprites.emplace_back(offset, dim);
    }

    return count;
  };

  uint32 sprite_pos = 0;
  for (const auto& content : json_data["content"]) {
    auto group_name = content["name"].get<std::string>();

    const uint32 group_size = parse_offsets(tex_info->width, tex_info->height, content["offset"]);
    parse_sequences(sprite_pos, group_name, content["anim"]);

    // _indices.reserve(group_size);
    size_t index_pos = _indices.size();
    for (uint32 i = 0; i < group_size; ++i) {
      _indices.emplace_back(i+sprite_pos);
    }
    _groups.emplace_back(
      std::move(group_name),
      vec_span{
        .index = static_cast<uint32>(index_pos),
        .count = static_cast<uint32>(_indices.size()-index_pos),
      }
    );
    sprite_pos += group_size;
  }
  _img_path = std::move(tex_path);

  return {};
}

std::string grid_atlas_loader::image_path() {
  return _img_path;
}

auto grid_atlas_loader::sprites() -> array_type<sprite_data> {
  return array_type<sprite_data>::from_vector(_sprites);
}

auto grid_atlas_loader::indices() -> array_type<atlas_index> {
  return array_type<atlas_index>::from_vector(_indices);
}

auto grid_atlas_loader::groups() -> array_type<sprite_data::group> {
  return array_type<sprite_data::group>::from_vector(_groups);
}

auto grid_atlas_loader::sequences() -> optional<array_type<sprite_data::sequence>> {
  if (_sequences.empty()) {
    return nullopt;
  }
  return array_type<sprite_data::sequence>::from_vector(_sequences);
}

} // namespace ntf
