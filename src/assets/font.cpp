#include "./font.hpp"

namespace ntf {

namespace {

struct ft2_things {
  FT_Library ft;
  FT_Face face;
};

} // namespace

ft2_bitmap_loader::ft2_bitmap_loader() noexcept :
  _has_face{false} {}

ft2_bitmap_loader::~ft2_bitmap_loader() noexcept {
  if (!_ft2_things) {
    return;
  }
  auto* things = static_cast<ft2_things*>(_ft2_things);
  FT_Done_FreeType(things->ft);
  delete static_cast<ft2_things*>(_ft2_things);
}

asset_expected<void> ft2_bitmap_loader::parse(const std::string& path, uvec2 pixel_size) {
  if (!_ft2_things) {
    _ft2_things = new ft2_things();
  }
}

} // namespace ntf
