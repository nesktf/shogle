#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "./texture.hpp"

namespace ntf {

optional<std::pair<extent2d, uint32>> stb_image_loader::_stbi_preparse_file(std::FILE* f) {
  int w, h, ch;
  if (stbi_info_from_file(f, &w, &h, &ch)) {
    return std::make_pair(
      extent2d{static_cast<uint32>(w), static_cast<uint32>(h)}, static_cast<uint32>(ch)
    );
  }
  return nullopt;
}

const char* stb_image_loader::_stbi_get_err() {
  return stbi_failure_reason();
}

uint8* stb_image_loader::_stbi_load_u8(std::FILE* f, image_load_flags flags, int32 desired) {
  stbi_set_flip_vertically_on_load(+(flags & image_load_flags::flip_y));
  int w, h, ch;
  return stbi_load_from_file(f, &w, &h, &ch, desired);
}

uint16* stb_image_loader::_stbi_load_u16(std::FILE* f, image_load_flags flags, int32 desired) {
  stbi_set_flip_vertically_on_load(+(flags & image_load_flags::flip_y));
  int w, h, ch;
  return stbi_load_from_file_16(f, &w, &h, &ch, desired);
}

float32* stb_image_loader::_stbi_load_f32(std::FILE* f, image_load_flags flags, int32 desired) {
  stbi_set_flip_vertically_on_load(+(flags & image_load_flags::flip_y));
  int w, h, ch;
  return stbi_loadf_from_file(f, &w, &h, &ch, desired);
}

void stb_image_loader::stbi_delete(void* data) {
  stbi_image_free(data);
}

} // namespace ntf
