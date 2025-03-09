#include "./atlas.hpp"
#include "./filesystem.hpp"

#define RET_ERR(msg, ...) \
  SHOGLE_LOG(error, "[ntf::grid_atlas_loader] " msg __VA_OPT__(,) __VA_ARGS__); \
  if constexpr (checked) { \
    return unexpected{asset_error::format({msg}__VA_OPT__(,) __VA_ARGS__)}; \
  } else { \
    SHOGLE_ASSET_THROW(msg __VA_OPT__(,) __VA_ARGS__); \
  }

#define RET_ERR_IF(cond, ...) \
  if (cond) { RET_ERR(__VA_ARGS__); }

using nlohmann::json;

namespace ntf {

namespace {

template<typename T>
using alloc = std::allocator<T>;

template<bool checked>
auto grid_atlas_loader_parse_impl(
  const std::string& path,
  atlas_load_flags flags
) -> std::conditional_t<checked,
                        asset_expected<grid_atlas_loader::data_t>,
                        grid_atlas_loader::data_t> {
  auto dir_path = file_dir(path);
  RET_ERR_IF(!dir_path, "Invalid path \"{}\"", path);

  std::ifstream f{path};
  RET_ERR_IF(!f.is_open(), "Failed to open json file \"{}\"", path);

  json json_data = json::parse(f, nullptr, false);
  RET_ERR_IF(json_data.is_discarded(), "Failed to parse json file \"{}\"", path);

  auto tex_path = fmt::format("{}/{}", *dir_path, json_data["file"].get<std::string>());
  auto tex_info = stb_image_loader::parse_meta(tex_path);
  RET_ERR_IF(!tex_info, "Failed to open texture file \"{}\"", tex_path);

  grid_atlas_loader::data_t atlas_data;
  try {
    size_t sprite_count = 0;
    size_t seq_count = 0;
    size_t group_count = json_data["content"].size();
    for (const auto& content : json_data["content"]) {
      sprite_count += content["offset"]["count"].get<size_t>();
      seq_count += content["anim"].size();
    }
    atlas_data.sprites.reset(alloc<sprite_data>{}.allocate(sprite_count), sprite_count);
    atlas_data.groups.reset(alloc<sprite_data::group>{}.allocate(group_count), group_count);
    atlas_data.sequences.reset(alloc<sprite_data::sequence>{}.allocate(seq_count), seq_count);
  } catch(const json::exception& ex) {
    RET_ERR("Failed to allocate atlas data: \"{}\"", ex.what());
  } catch(const std::exception& ex) {
    RET_ERR("Failed to allocate atlas data: \"{}\"", ex.what());
  } catch(...) {
    RET_ERR("Unknown error");
  }

  SHOGLE_LOG(debug,
             "[ntf::grid_atlas_loader] Found {} sprites, {} groups, {} sequences for \"{}\"",
             atlas_data.sprites.size(),
             atlas_data.groups.size(),
             atlas_data.sequences.size(),
             path);

  try {
    uint32 sprite_count = 0;
    uint32 sequence_count = 0;
    uint32 group_count = 0;
    auto parse_sequences = [&](const std::string& group_name, const json& entry) {
      for (const auto& seq_entry : entry) {
        const auto& seq = seq_entry["sequence"];
        const uint32 delay = seq_entry["delay"].get<uint32>();
        const uint32 total_frames = seq.size()*delay;

        size_t index_pos = atlas_data.indices.size();
        atlas_data.indices.resize(index_pos+total_frames, 0);
        for (uint32 i = 0, curr_frame = 0; i < total_frames; i += delay, ++curr_frame) {
          uint32 index = seq[curr_frame];
          for (uint32 j = 0; j < delay; ++j) {
            atlas_data.indices[index_pos+j+i] = sprite_count+index;
          }
        }

        std::construct_at(atlas_data.sequences.get()+sequence_count,
                          fmt::format("{}.{}", group_name, seq_entry["name"].get<std::string>()),
                          vec_span{
                            .index = static_cast<uint32>(index_pos),
                            .count = total_frames,
                          });
        ++sequence_count;
      }
      // return entry.size();
    };
    auto parse_offsets = [&](uint32 tex_w, uint32 tex_h, const json& entry) {
      const bool flip_x = +(flags & atlas_load_flags::flip_x);
      const bool flip_y = +(flags & atlas_load_flags::flip_y);

      const uint32 count = entry["count"].get<uint32>();

      const uint32 x0 = entry["x0"].get<uint32>();
      const uint32 y0 = entry["y0"].get<uint32>();
      const uint32 dx = entry["dx"].get<uint32>();
      const uint32 dy = entry["dy"].get<uint32>();

      const uint32 cols = entry["cols"].get<uint32>();
      const float32 row_ratio = static_cast<float32>(count)/static_cast<float32>(cols);
      const uint32 rows = static_cast<uint32>(std::ceil(row_ratio));

      const uvec2 dim{dx/cols, dy/rows}; // all the sprites in the group have the same size
      vec2 uv_scale = static_cast<vec2>(dim);
      uv_scale.x /= (flip_x*-1.f + !flip_x*1.f)*static_cast<float32>(tex_w);
      uv_scale.y /= (flip_y*-1.f + !flip_y*1.f)*static_cast<float32>(tex_h);

      for (uint32 i = 0; i < count; ++i) {
        const uint32 row = i / cols;
        const uint32 col = i % cols;

        const float32 uv_offset_x =
          static_cast<float32>((x0*cols) + (col*dx))/static_cast<float32>(tex_w*cols);
        const float32 uv_offset_y = 
          static_cast<float32>((y0*rows) + (row*dy))/static_cast<float32>(tex_h*rows);

        std::construct_at(atlas_data.sprites.get()+sprite_count,
                          vec4{
                            uv_scale.x,
                            uv_scale.y,
                            flip_x*(1.f - uv_offset_x) + !flip_x*uv_offset_x,
                            flip_y*(1.f - uv_offset_y) + !flip_y*uv_offset_y
                          },
                          dim);
        ++sprite_count;
      }

      return count;
    };

    for (const auto& content : json_data["content"]) {
      auto group_name = content["name"].get<std::string>();

      parse_sequences(group_name, content["anim"]);
      const uint32 group_sz = parse_offsets(tex_info->width, tex_info->height, content["offset"]);

      // _indices.reserve(group_size);
      size_t index_count = atlas_data.indices.size();
      for (uint32 i = 0; i < group_sz; ++i) {
        atlas_data.indices.emplace_back(i+sprite_count-group_sz);
      }
      std::construct_at(atlas_data.groups.get()+group_count,
                        std::move(group_name),
                        vec_span{
                          .index = static_cast<uint32>(index_count),
                          .count = group_sz,
                        });
      // sprite_count += group_sz;
      // sequence_count += group_seqs;
      ++group_count;
    }
    atlas_data.img_path = std::move(tex_path);
  } catch (const json::exception& ex) {
    RET_ERR("Failed to parse json: \"{}\"", ex.what());
  } catch (const std::exception& ex) {
    RET_ERR("Failed to parse json: \"{}\"", ex.what());
  } catch (...) {
    RET_ERR("Unknown error");
  }

  if constexpr (checked) {
    return {std::move(atlas_data)};
  } else {
    return atlas_data;
  }
}

} // namespace

auto grid_atlas_loader::parse(
  const std::string& path,
  atlas_load_flags flags
) -> asset_expected<data_t> {
  return grid_atlas_loader_parse_impl<true>(path, flags);
}

auto grid_atlas_loader::parse(
  unchecked_t,
  const std::string& path,
  atlas_load_flags flags
) -> data_t {
  return grid_atlas_loader_parse_impl<false>(path, flags);
}

std::string grid_atlas_loader::image_path(data_t& data) {
  NTF_ASSERT(!data.img_path.empty());
  return std::move(data.img_path);
}

auto grid_atlas_loader::sprites(data_t& data) -> array_type<sprite_data> {
  NTF_ASSERT(data.sprites);
  return std::move(data.sprites);
}

auto grid_atlas_loader::groups(data_t& data) -> array_type<sprite_data::group> {
  NTF_ASSERT(data.groups);
  return std::move(data.groups);
}

auto grid_atlas_loader::sequences(data_t& data) -> optional<array_type<sprite_data::sequence>> {
  if (data.sequences.size() == 0) {
    return nullopt;
  }
  return std::move(data.sequences);
}

auto grid_atlas_loader::indices(data_t& data) -> array_type<atlas_index> {
  NTF_ASSERT(!data.indices.empty());
  return array_type<atlas_index>::from_container(data.indices);
}

} // namespace ntf
