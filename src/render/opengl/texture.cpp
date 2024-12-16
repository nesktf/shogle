#include "texture.hpp"

namespace ntf {

uint32 gl_texture_type(r_texture_type type, uint32 count) {
  switch (type) {
    case r_texture_type::texture1d: return count > 1 ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case r_texture_type::texture2d: return count > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case r_texture_type::texture3d: return GL_TEXTURE_3D;
    case r_texture_type::cubemap:   return GL_TEXTURE_CUBE_MAP;

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

GLuint gl_texture::_allocate(uint32 gltype, uint32 glformat, uint32 count, 
                             uint32 mipmaps, uvec3 dim) {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(gltype, id);

  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(dim.x > 0);
      NTF_ASSERT(count > 0);
      glTexStorage2D(gltype, mipmaps, glformat, dim.x, count);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(dim.x > 0);
      glTexStorage1D(gltype, mipmaps, glformat, dim.x);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0);
      NTF_ASSERT(count > 0);
      glTexStorage3D(gltype, mipmaps, glformat, dim.x, dim.y, count);
      break;
    }

    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(dim.x == dim.y);
      [[fallthrough]];
    }
    case GL_TEXTURE_2D: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0);
      glTexStorage2D(gltype, mipmaps, glformat, dim.x, dim.y);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0 && dim.z > 0);
      glTexStorage3D(gltype, mipmaps, glformat, dim.x, dim.y, dim.z);
      break;
    };

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };

  return id;
}

void gl_texture::_upload(uint32 gltype, uint32 glformat,
                         const uint8* texels, uint32 mipmap, uint32 index, uvec3 offset) {
  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < _dim.x);
      NTF_ASSERT(index < _count);
      glTexSubImage2D(gltype, mipmap, offset.x, index, _dim.x, _count, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < _dim.x);
      glTexSubImage1D(gltype, mipmap, offset.x, _dim.x, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < _dim.x && offset.y < _dim.y);
      NTF_ASSERT(index < _count);
      glTexSubImage3D(gltype, mipmap, offset.x, offset.y, index, _dim.x, _dim.y, _count, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < _dim.x && offset.y < _dim.y);
      glTexSubImage2D(gltype, mipmap, offset.x, offset.y, _dim.x, _dim.y, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < _dim.x);
      const uint32 glface = GL_TEXTURE_CUBE_MAP_POSITIVE_X+index; 
      glTexSubImage2D(glface, mipmap, offset.x, offset.y, _dim.x, _dim.y, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < _dim.x && offset.y < _dim.y && offset.z < _dim.z);
      glTexSubImage3D(gltype, mipmap, offset.x, offset.y, offset.z, _dim.x, _dim.y, _dim.z,
                      glformat, GL_UNSIGNED_BYTE, texels);
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
}

void gl_texture::_set_sampler(uint32 gltype, uint32 glsamplermin, uint32 glsamplermag) {
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glsamplermag);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glsamplermin);
}

void gl_texture::_set_addressing(uint32 gltype, uint32 gladdress) {
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, gladdress); // U
  if (gltype != GL_TEXTURE_1D || gltype != GL_TEXTURE_1D_ARRAY) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_T, gladdress); // V
    if (gltype == GL_TEXTURE_3D || gltype == GL_TEXTURE_CUBE_MAP) {
      glTexParameteri(gltype, GL_TEXTURE_WRAP_R, gladdress); // W (?)
    }
  }
}


void gl_texture::load(r_texture_type type, r_texture_format format,
                      r_texture_sampler sampler, r_texture_address addressing,
                      const uint8** texels, uint32 mipmaps, uint32 count, uvec3 dim) {
  NTF_ASSERT(!_id);

  // TODO: Move the checks to the context
  if (!count) {
    return;
  }

  if (count > 1 && type == r_texture_type::texture3d) {
    return;
  }

  if (count != 6 && type == r_texture_type::cubemap) {
    return;
  }

  // TODO: Validate dimensions

  const uint32 gltype = gl_texture_type(type, count);
  NTF_ASSERT(gltype);

  const uint32 glformat = gl_texture_format(format);
  NTF_ASSERT(glformat);

  const uint32 glsamplermin = gl_texture_sampler(sampler, (mipmaps > 0));
  const uint32 glsamplermag = gl_texture_sampler(sampler, false); // magnification doesn't use mips
  NTF_ASSERT(glsamplermin && glsamplermag);

  const uint32 gladdress = gl_texture_address(addressing);
  NTF_ASSERT(gladdress);

  if (!(_id = _allocate(gltype, glformat, count, mipmaps, dim))) { // binds the texture
    glBindTexture(gltype, 0);
    return;
  }

  _mipmaps = mipmaps;
  _count = count;
  _dim = dim;
  _type = type;
  _addressing = addressing;
  _sampler = sampler;
  _format = format; // internal format

  _set_sampler(gltype, glsamplermin, glsamplermag);
  _set_addressing(gltype, gladdress);
  if (texels) {
    for (uint32 i = 0; i < count; ++i) {
      _upload(gltype, glformat, texels[i], 0, i, uvec3{0, 0, 0});
    }
    if (_mipmaps > 0) {
      glGenerateMipmap(gltype);
    }
  }

  glBindTexture(gltype, 0);
}

void gl_texture::unload() {
  NTF_ASSERT(_id);

  glDeleteTextures(1, &_id);

  _id = 0;
  _dim = uvec3{0, 0, 0};
  _type = r_texture_type::none;
  _addressing = r_texture_address::none;
  _sampler = r_texture_sampler::none;
  _format = r_texture_format::none;
  _count = 0;
  _mipmaps = 0;
}

void gl_texture::sampler(r_texture_sampler sampler) {
  NTF_ASSERT(_id);

  const uint32 gltype = gl_texture_type(_type, _count);
  NTF_ASSERT(gltype);

  const uint32 glsamplermin = gl_texture_sampler(sampler, (_mipmaps > 0));
  const uint32 glsamplermag = gl_texture_sampler(sampler, false); // magnification doesn't use mips
  NTF_ASSERT(glsamplermag && glsamplermin);

  glBindTexture(gltype, _id);
  _set_sampler(gltype, glsamplermin, glsamplermag);
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
  _set_addressing(gltype, gladdress);
  glBindTexture(gltype, 0);

  _addressing = address;
}

void gl_texture::data(r_texture_format format, const uint8* texels, uint32 index, uvec3 offset) {
  NTF_ASSERT(_id);

  const uint32 gltype = gl_texture_type(_type, _count);
  NTF_ASSERT(gltype);

  const uint32 glformat = gl_texture_format(format);
  NTF_ASSERT(glformat);

  glBindTexture(gltype, _id);
  _upload(gltype, glformat, texels, 0, index, offset);
  if (_mipmaps > 0) {
    glGenerateMipmap(gltype);
  }
  glBindTexture(gltype, 0);
}

} // namespace ntf
