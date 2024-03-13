#include "resources/texture_data.hpp"
#include "core/logger.hpp"

#include "stb/stb_image.h"

namespace ntf::shogle {

TextureData::TextureData(const char* path, GLenum tex_dim, aiTextureType ai_type, Type tex_type) :
  path(path),
  tex_dim(tex_dim),
  ai_type(ai_type),
  tex_type(tex_type) {
  data = stbi_load(path, &width, &height, &nr_channels, 0);
  if (!data) {
    logger::fatal("[TextureData] File not found: {}", path);
  }
}

TextureData::~TextureData() {
  stbi_image_free(data);
}


} // namespace ntf::shogle
