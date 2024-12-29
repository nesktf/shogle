#pragma once

#include "../math/matrix.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#endif

namespace ntf {

using color3 = vec3;
using color4 = vec4;

enum class r_api : uint8 {
  none = 0,
  software,
  opengl,
  vulkan,
};

enum class r_win_api : uint8 {
  glfw,
  sdl2, // ?
};
class r_window;

enum class r_resource_type : uint8 {
  buffer,
  texture,
  pipeline,
  shader,
  framebuffer,
};

using r_handle_value = uint64;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

template<typename RenderCtx, typename T, typename HandleView>
class r_handle {
public:
  r_handle() = default;

private:
  r_handle(RenderCtx& ctx, r_handle_value handle) :
    _ctx(&ctx), _handle(handle) {};

public:
  const T& get() const { 
    NTF_ASSERT(_ctx);
    return _ctx->resource(std::type_identity<T>{}, _handle);
  }

  T& get() {
    NTF_ASSERT(_ctx);
    return _ctx->resource(std::type_identity<T>{}, _handle);
  }

  void reset() { 
    if (_ctx) {
      _ctx->destroy(std::type_identity<T>{}, _handle);
    }
    _ctx = nullptr;
    _handle = r_handle_tombstone;
  }

  r_handle_value handle() const { return _handle; }

  T& operator*() { return get(); }
  const T& operator*() const { return get(); }
  T* operator->() { return &get(); }
  const T* operator->() const { return &get(); }

  explicit operator bool() const { return _ctx != nullptr && _handle != r_handle_tombstone; }
  operator HandleView() const { return HandleView{RenderCtx::RENDER_API, _handle}; }

private:
  RenderCtx* _ctx{nullptr};
  r_handle_value _handle{r_handle_tombstone};

public:
  ~r_handle() noexcept {
    if (_ctx) {
      _ctx->destroy(std::type_identity<T>{}, _handle);
    }
  }

  r_handle(const r_handle&) = delete;
  r_handle& operator=(const r_handle&) = delete;

  r_handle(r_handle&& h) noexcept :
    _ctx(std::move(h._ctx)), _handle(std::move(h._handle)) { h._ctx = nullptr; }
  r_handle& operator=(r_handle&& h) noexcept {
    if (std::addressof(h) == this) {
      return *this;
    }

    if (_ctx) {
      _ctx->destroy(std::type_identity<T>{}, _handle);
    }

    _ctx = std::move(h._ctx);
    _handle = std::move(h._handle);

    h._ctx = nullptr;

    return *this;
  }

private:
  friend RenderCtx;
};


enum class r_attrib_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

constexpr inline size_t r_attrib_type_size(r_attrib_type type) {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t float_sz = sizeof(float);
  constexpr size_t double_sz = sizeof(double);

  switch (type) {
    case r_attrib_type::i32:   return int_sz;
    case r_attrib_type::ivec2: return 2*int_sz;
    case r_attrib_type::ivec3: return 3*int_sz;
    case r_attrib_type::ivec4: return 4*int_sz;

    case r_attrib_type::f32:   return float_sz;
    case r_attrib_type::vec2:  return 2*float_sz;
    case r_attrib_type::vec3:  return 3*float_sz;
    case r_attrib_type::vec4:  return 4*float_sz;
    case r_attrib_type::mat3:  return 9*float_sz;
    case r_attrib_type::mat4:  return 16*float_sz;

    case r_attrib_type::f64:   return double_sz;
    case r_attrib_type::dvec2: return 2*double_sz;
    case r_attrib_type::dvec3: return 3*double_sz;
    case r_attrib_type::dvec4: return 4*double_sz;
    case r_attrib_type::dmat3: return 9*double_sz;
    case r_attrib_type::dmat4: return 16*double_sz;
  };

  return 0;
};

constexpr inline uint32 r_attrib_type_dim(r_attrib_type type) {
  switch (type) {
    case r_attrib_type::i32:   [[fallthrough]];
    case r_attrib_type::f32:   [[fallthrough]];
    case r_attrib_type::f64:   return 1;

    case r_attrib_type::vec2:  [[fallthrough]];
    case r_attrib_type::ivec2: [[fallthrough]];
    case r_attrib_type::dvec2: return 2;

    case r_attrib_type::vec3:  [[fallthrough]];
    case r_attrib_type::ivec3: [[fallthrough]];
    case r_attrib_type::dvec3: return 3;

    case r_attrib_type::vec4:  [[fallthrough]];
    case r_attrib_type::ivec4: [[fallthrough]];
    case r_attrib_type::dvec4: return 4;

    case r_attrib_type::mat3:  [[fallthrough]];
    case r_attrib_type::dmat3: return 9;

    case r_attrib_type::mat4:  [[fallthrough]];
    case r_attrib_type::dmat4: return 16;
  };

  return 0;
}

struct r_attrib_descriptor {
  uint32        binding;
  uint32        location;
  size_t        offset;
  r_attrib_type type;
};

struct r_uniform_descriptor {
  uint32        location;
  r_attrib_type type;
  const void*   data;
};

enum class r_shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

enum class r_shader_usage_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(r_shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(r_shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(r_shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(r_shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(r_shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(r_shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_shader_usage_flag)

struct r_shader_descriptor {
  std::string_view  source;
  r_shader_type     type;
};

enum class r_texture_type : uint8 {
  texture1d = 0,
  texture1d_array,
  texture2d,
  texture2d_array,
  texture3d,
  cubemap,
};
constexpr uint8 r_texture_type_count = 6;

enum class r_texture_format : uint8 {
  mono = 0,
  rgb,
  rgba,
};

enum class r_texture_sampler : uint8 {
  nearest = 0,
  linear,
};

enum class r_texture_address : uint8 {
  repeat = 0,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class r_cubemap_face : uint8 {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
};
constexpr uint8 r_cubemap_face_count = 6;

struct r_texture_descriptor {
  const uint8* const* texels;
  uint32              count;
  uint32              mipmap_level;
  uvec3               extent;
  r_texture_type      type;
  r_texture_format    format;
  r_texture_sampler   sampler;
  r_texture_address   addressing;
};

enum class r_buffer_type : uint8 {
  vertex,
  index,
  texel,
  uniform,
  shader_storage,
};
constexpr uint8 r_buffer_type_count = 5;

struct r_buffer_descriptor {
  const void*   data;
  size_t        size;
  r_buffer_type type;
};

enum class r_primitive : uint8 {
  triangles,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class r_polygon_mode : uint8 {
  point,
  line,
  fill,
};

struct r_framebuffer_descriptor {
  uvec4 viewport;
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_pipeline_descriptor {
  const r_handle_value*       stages;
  uint32                      stage_count;
  const r_attrib_descriptor*  attribs;
  uint32                      attrib_count;
  r_primitive                 primitive;
  r_polygon_mode              poly_mode;
};


enum class r_clear_flag : uint8 {
  none    = 0,
  color   = 1 << 0,
  depth   = 1 << 1,
  stencil = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear_flag)

struct r_draw_cmd {
  r_handle_value vertex_buffer;
  r_handle_value index_buffer;
  r_handle_value pipeline;
  r_handle_value framebuffer;

  const r_handle_value*       textures;
  uint32                      texture_count;
  const r_uniform_descriptor* uniforms;
  uint32                      uniform_count;

  uint32                      draw_count;
  uint32                      draw_offset;
  uint32                      instance_count;
};

template<typename T>
concept render_context_object = requires(T ctx) {
  { ctx.start_frame() } -> std::convertible_to<void>;
  { ctx.end_frame() } -> std::convertible_to<void>;
  { ctx.enqueue(r_draw_cmd{}) } -> std::convertible_to<void>;
  { ctx.device_wait() } -> std::convertible_to<void>;
};

} // namespace ntf

#undef SHOGLE_DECLARE_RENDER_HANDLE
