#include "texture.hpp"

namespace ntf {

uint32 gl_texture_type(r_texture_type type, uint32 count) {
  switch (type) {
    case r_texture_type::texture1d: return count > 1 ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case r_texture_type::texture2d: return count > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case r_texture_type::texture3d: return GL_TEXTURE_3D;
    case r_texture_type::cubemap:   return GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    case r_texture_type::none:      return 0;
  };
  NTF_UNREACHABLE();
}

uint32 gl_texture_format(r_texture_format format) {
  switch (format) {
    case r_texture_format::mono:    return GL_RED;
    case r_texture_format::rgb:     return GL_RGB;
    case r_texture_format::rgba:    return GL_RGBA;

    case r_texture_format::none:    return 0;
  };
  NTF_UNREACHABLE();
}

uint32 gl_texture_sampler(r_texture_sampler sampler, bool mipmaps) {
  switch (sampler) {
    case r_texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case r_texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    // TODO: change this?
    // case r_texture_sampler::linear: {
    //   if (mipmaps) {
    //     if (lin_lvl) {
    //       return lin_lvls ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    //     } else {
    //       return lin_lvl ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
    //     }
    //   } else {
    //     return GL_LINEAR;
    //   }
    //   break;
    // }

    case r_texture_sampler::none:     return 0;
  }
  NTF_UNREACHABLE();
}

uint32 gl_texture_address(r_texture_address address) {
  switch (address) {
    case r_texture_address::clamp_border:         return GL_CLAMP_TO_BORDER;
    case r_texture_address::repeat:               return GL_REPEAT;
    case r_texture_address::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case r_texture_address::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case r_texture_address::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;

    case r_texture_address::none:                 return 0;
  };
  NTF_UNREACHABLE();
};

bool gl_texture::load(const uint8** data, uint32 count, uint32 mipmaps, uvec3 dim,
                      r_texture_type type, r_texture_format format,
                      r_texture_sampler sampler, r_texture_address address) {
  NTF_ASSERT(!_id);

  // TODO: Move the checks to the context
  if (!data || !count) {
    return false;
  }

  if (count > 1 && type == r_texture_type::texture3d) {
    return false;
  }

  if (count != 6 && type == r_texture_type::cubemap) {
    return false;
  }

  const uint32 gltype = gl_texture_type(type, count);
  NTF_ASSERT(gltype);

  const uint32 glformat = gl_texture_format(format);
  NTF_ASSERT(glformat);

  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(gltype, id);

  switch (type) {
    case r_texture_type::texture1d: {
      if (count > 1) {
        glTexStorage2D(gltype, mipmaps, glformat, dim.x, count);
        for (uint32 i = 0; i < count; ++i) {
          glTexSubImage2D(gltype, mipmaps, 0, i, dim.x, count, glformat,
                          GL_UNSIGNED_BYTE, data[i]);
        }
      } else {
        glTexImage1D(gltype, mipmaps, glformat, dim.x, 0, glformat,
                     GL_UNSIGNED_BYTE, data[0]);
      }
      break;
    };
    case r_texture_type::texture2d: {
      if (count > 1) {
        glTexStorage3D(gltype, mipmaps, glformat, dim.x, dim.y, count);
        for (uint32 i = 0; i < count; ++i) {
          glTexSubImage3D(gltype, mipmaps, 0, 0, i, dim.x, dim.y, count, glformat,
                          GL_UNSIGNED_BYTE, data[i]);
        }
      } else {
        glTexImage2D(gltype, mipmaps, glformat, dim.x, dim.y, 0, glformat,
                     GL_UNSIGNED_BYTE, data[0]);
      }
      break;
    };
    case r_texture_type::texture3d: {
      glTexImage3D(gltype, mipmaps, glformat, dim.x, dim.y, dim.z, 0, glformat,
                   GL_UNSIGNED_BYTE, data[0]);
      break;
    };
    case r_texture_type::cubemap: {
      for (uint32 i = 0; i < count; ++i) {
        const uint8* texels = data[i];
        glTexImage2D(glformat+i, mipmaps, glformat, dim.x, dim.y, 0, glformat,
                     GL_UNSIGNED_BYTE, texels);
      }
      break;
    }
    case r_texture_type::none:
      NTF_UNREACHABLE();
  };

  const uint32 glsampler = gl_texture_sampler(sampler, (mipmaps > 0));
  const uint32 gladdress = gl_texture_address(address);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glsampler);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glsampler);

  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, gladdress); // U
  if (type != r_texture_type::texture1d) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_T, gladdress); // V
    if (type != r_texture_type::texture2d) {
      glTexParameteri(gltype, GL_TEXTURE_WRAP_R, gladdress); // W?
    }
  }

  if (mipmaps > 0) {
    glGenerateMipmap(gltype);
  }

  glBindTexture(gltype, 0);

  _id = id;
  _dim = dim;
  _type = type;
  _addressing = address;
  _sampler = sampler;
  _count = count;
  _mipmaps = mipmaps;

  return true;
}

void gl_texture::unload() {
  NTF_ASSERT(_id);
  glDeleteTextures(1, &_id);
  _id = 0;
  _dim = uvec3{0, 0, 0};
  _type = r_texture_type::none;
  _addressing = r_texture_address::none;
  _sampler = r_texture_sampler::none;
  _count = 0;
  _mipmaps = 0;
}

void gl_texture::sampler(r_texture_sampler sampler) {
  NTF_ASSERT(_id);

  const uint32 gltype = gl_texture_type(_type, _count);
  NTF_ASSERT(gltype);
  const uint32 glsampler = gl_texture_sampler(sampler, (_mipmaps > 0));
  NTF_ASSERT(glsampler);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glsampler);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glsampler);
  glBindTexture(gltype, 0);

  _sampler = sampler;
}

void gl_texture::addressing(r_texture_address address) {
  NTF_ASSERT(_id);

  const uint32 gltype = gl_texture_type(_type, _count);
  NTF_ASSERT(gltype);
  const uint32 gladdress = gl_texture_address(address);
  NTF_ASSERT(gladdress);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, gladdress); // U
  if (_type != r_texture_type::texture1d) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_T, gladdress); // V
    if (_type != r_texture_type::texture2d) {
      glTexParameteri(gltype, GL_TEXTURE_WRAP_R, gladdress); // W?
    }
  }
  glBindTexture(gltype, 0);

  _addressing = address;
}

} // namespace ntf
