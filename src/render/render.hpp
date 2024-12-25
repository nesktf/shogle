#pragma once

#include "../math/matrix.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#endif

#define SHOGLE_DECLARE_RENDER_HANDLE(__name) \
struct __name { \
public: \
  __name() = default; \
  __name(r_api api_, r_handle_value handle_) : \
    api(api_), handle(handle_) {} \
public: \
  explicit operator bool() const { return handle != r_handle_tombstone && api != r_api::none; } \
public: \
  r_api api{r_api::none}; \
  r_handle_value handle{r_handle_tombstone}; \
}

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
  none = 0,
  glfw,
  sdl2, // ?
};
class r_window;

enum class r_resource_type : uint8 {
  none = 0,
  buffer,
  texture,
  pipeline,
  shader,
  framebuffer,
};

using r_handle_value = uint64;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

SHOGLE_DECLARE_RENDER_HANDLE(r_buffer_view);
SHOGLE_DECLARE_RENDER_HANDLE(r_texture_view);
SHOGLE_DECLARE_RENDER_HANDLE(r_pipeline_view);
SHOGLE_DECLARE_RENDER_HANDLE(r_shader_view);
SHOGLE_DECLARE_RENDER_HANDLE(r_framebuffer_view);

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
  none = 0,
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

    case r_attrib_type::none:  return 0;
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

    case r_attrib_type::none:  return 0;
  };

  return 0;
}

struct r_attrib_descriptor {
  uint32        binding{0};
  uint32        location{0};
  size_t        offset{0};
  r_attrib_type type{r_attrib_type::none};
};

struct r_uniform_descriptor {
  uint32        location{0};
  r_attrib_type type{r_attrib_type::none};
  const void*   data{nullptr};
};

enum class r_shader_type : uint8 {
  none                = 0,
  vertex              = 1 << 0,
  fragment            = 1 << 1,
  geometry            = 1 << 2,
  tesselation_eval    = 1 << 3,
  tesselation_control = 1 << 4,
  compute             = 1 << 5,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_shader_type)

struct r_shader_descriptor {
  std::string_view  source{};
  r_shader_type     type{r_shader_type::none};
};

enum class r_texture_type : uint8 {
  none = 0,
  texture1d,
  texture2d,
  texture3d,
  cubemap,
};

enum class r_texture_format : uint8 {
  none = 0,
  mono,
  rgb,
  rgba,
};

enum class r_texture_sampler : uint8 {
  none = 0,
  nearest,
  linear,
};

enum class r_texture_address : uint8 {
  none = 0,
  repeat,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class r_cubemap_face {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
  count,
};

struct r_texture_descriptor {
  const uint8* const* texels{nullptr};
  uint32              count{0};
  uint32              mipmap_level{0};
  uvec3               extent{};
  r_texture_type      type{r_texture_type::none};
  r_texture_format    format{r_texture_format::none};
  r_texture_sampler   sampler{r_texture_sampler::none};
  r_texture_address   addressing{r_texture_address::none};
};

enum class r_buffer_type : uint8 {
  none = 0,
  vertex,
  index,
  uniform,
};

struct r_buffer_descriptor {
  const void*   data{nullptr};
  size_t        size{0};
  r_buffer_type type{r_buffer_type::none};
};

enum class r_primitive : uint8 {
  none = 0,
  triangles,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class r_polygon_mode : uint8 {
  none = 0,
  point,
  line,
  fill,
};

struct r_framebuffer_descriptor {
  uvec4 viewport{};
  r_texture_sampler sampler{r_texture_sampler::none};
  r_texture_address addressing{r_texture_address::none};
};

struct r_pipeline_descriptor {
  const r_shader_view*        stages{nullptr};
  uint32                      stage_count{0};
  const r_attrib_descriptor*  attribs{nullptr};
  uint32                      attrib_count{0};
  r_primitive                 primitive{r_primitive::none};
  r_polygon_mode              poly_mode{r_polygon_mode::none};
};


enum class r_clear : uint8 {
  none    = 0,
  color   = 1 << 0,
  depth   = 1 << 1,
  stencil = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear)

struct r_draw_cmd {
  r_buffer_view               vertex_buffer{};
  r_buffer_view               index_buffer{};
  r_pipeline_view             pipeline{};
  r_framebuffer_view          framebuffer{};

  const r_texture_view*       textures{nullptr};
  uint32                      texture_count{0};
  const r_uniform_descriptor* uniforms{nullptr};
  uint32                      uniform_count{0};

  r_primitive                 primitive{r_primitive::none};
  uint32                      draw_count{0};
  uint32                      draw_offset{0};
  uint32                      instance_count{0};
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
