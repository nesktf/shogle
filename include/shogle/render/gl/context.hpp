#pragma once

#include <shogle/render/gl/common.hpp>
#include <shogle/render/gl/pipeline.hpp>

#include <shogle/util/function.hpp>

namespace shogle {

struct gl_clear_opts {
public:
  enum clear_flag : gldefs::GLenum {
    CLEAR_NONE = 0x00000000,    // GL_NONE
    CLEAR_COLOR = 0x00004000,   // GL_COLOR_BUFFER_BIT
    CLEAR_DEPTH = 0x00000100,   // GL_DEPTH_BUFFER_BIT
    CLEAR_STENCIL = 0x00000400, // GL_STENCIL_BUFFER_BIT
  };

  struct fbo_initializer {
    color4 clear_color;
    rectangle_pos<u32> viewport;
    gldefs::GLenum clear_flags;
    gldefs::GLhandle fbo;
  };

public:
  color4 clear_color;
  optional<rectangle_pos<u32>> viewport;
  gldefs::GLenum clear_flags;
  span<const fbo_initializer> fbos;
};

class gl_clear_builder {
public:
  gl_clear_builder() noexcept;

public:
  gl_clear_builder& set_viewport(const rectangle_pos<u32>& viewport) &;
  gl_clear_builder& set_viewport(u32 x, u32 y, u32 width, u32 height) &;
  gl_clear_builder& set_clear_color(const color4& color);
  gl_clear_builder& set_clear_color(f32 r, f32 g, f32 b, f32 a = 1.f) &;
  gl_clear_builder& set_clear_flag(gl_clear_opts::clear_flag flag) &;

  gl_clear_builder& add_framebuffer(const gl_framebuffer& fbo) &;
  gl_clear_builder& set_fb_viewport(size_t idx, const rectangle_pos<u32>& viewport) &;
  gl_clear_builder& set_fb_viewport(size_t idx, u32 x, u32 y, u32 width, u32 height) &;
  gl_clear_builder& set_fb_clear_color(size_t idx, const color4& color) &;
  gl_clear_builder& set_fb_clear_color(size_t idx, f32 r, f32 g, f32 b, f32 a = 1.f) &;
  gl_clear_builder& set_fb_clear_flag(size_t idx, gl_clear_opts::clear_flag flag) &;

public:
  void reset() &;
  gl_clear_opts build() const&;

private:
  color4 _color;
  optional<rectangle_pos<u32>> _viewport;
  gldefs::GLenum _clear_flags;
  std::vector<gl_clear_opts::fbo_initializer> _fbos;
};

struct gl_push_uniform {
  template<::shogle::meta::attribute_type T>
  gl_push_uniform(u32 location_, const T& data_) :
      type(meta::attribute_traits<T>::tag), location(location_) {
    std::memcpy(&data[0], &data_, sizeof(T));
  }

  template<typename T>
  requires(std::is_trivially_copyable_v<T>)
  gl_push_uniform(u32 location_, const T& data_, attribute_type type_) :
      type(type_), location(location_) {
    std::memcpy(&data[0], &data_, sizeof(T));
  }

  alignas(::shogle::mat4) u8 data[sizeof(::shogle::mat4)];
  attribute_type type;
  u32 location;
};

struct gl_draw_cmd {
public:
  struct texture_binding {
    gldefs::GLhandle texture;
    gldefs::GLenum type;
    u32 index;
  };

  struct shader_binding {
    gldefs::GLhandle buffer;
    gldefs::GLenum type;
    size_t size;
    size_t offset;
    u32 location;
  };

public:
  optional<inplace_trivial_fn<void(), 2 * sizeof(void*)>> on_render;
  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_pipeline> pipeline;
  span<const shader_binding> shader_bindings;
  span<const texture_binding> texture_bindings;
  span<const gl_push_uniform> uniforms;
  optional<rectangle_pos<u32>> viewport;
  optional<rectangle_pos<u32>> scissor;
  u32 draw_count;
  u32 instances;
};

class gl_cmd_builder {
public:
  gl_cmd_builder() noexcept;

public:
  gl_cmd_builder& set_vertex_layout(const gl_vertex_layout& layout) &;
  gl_cmd_builder& set_pipeline(const gl_pipeline& pipeline) &;

  gl_cmd_builder& set_viewport(const rectangle_pos<u32>& viewport) &;
  gl_cmd_builder& set_viewport(u32 x, u32 y, u32 width, u32 height) &;
  gl_cmd_builder& set_scissor(const rectangle_pos<u32>& scissor) &;
  gl_cmd_builder& set_scissor(u32 x, u32 y, u32 width, u32 height) &;

  gl_cmd_builder& set_instances(u32 instances) &;
  gl_cmd_builder& set_draw_count(u32 count) &;

  gl_cmd_builder& add_shader_buffer(u32 location, const gl_buffer& buffer, size_t size = 0,
                                    size_t offset = 0) &;
  gl_cmd_builder& add_texture(const gl_texture& texture, u32 location) &;

  template<::shogle::meta::attribute_type T>
  gl_cmd_builder& add_uniform(const T& value, u32 location) & {
    _uniforms.emplace_back(location, value);
    return *this;
  }

  template<typename T>
  requires(std::is_trivially_copyable_v<T>)
  gl_cmd_builder& add_uniform(const T& value, u32 location, attribute_type tag) & {
    _uniforms.emplace_back(location, value, tag);
    return *this;
  }

  template<typename F>
  gl_cmd_builder& set_callback(F&& func) & {
    if (_on_render.has_value()) {
      _on_render.reset();
    }
    _on_render.emplace(std::forward<F>(func));
    return *this;
  }

  template<typename F, typename... Args>
  gl_cmd_builder& set_callback(in_place_type_t<F>, Args&&... args) & {
    if (_on_render.has_value()) {
      _on_render.reset();
    }
    _on_render.emplace(in_place_type<F>, std::forward<Args>(args)...);
    return *this;
  }

public:
  void reset() &;
  gl_draw_cmd build() const&;

private:
  optional<inplace_trivial_fn<void(), 2 * sizeof(void*)>> _on_render;
  ptr_view<const gl_vertex_layout> _vertex_layout;
  ptr_view<const gl_pipeline> _pipeline;
  std::vector<gl_draw_cmd::shader_binding> _shader_binds;
  std::vector<gl_draw_cmd::texture_binding> _texture_binds;
  std::vector<gl_push_uniform> _uniforms;
  optional<rectangle_pos<u32>> _viewport;
  optional<rectangle_pos<u32>> _scissor;
  u32 _draw_count;
  u32 _instances;
};

struct gl_ext_cmd {
  inplace_trivial_fn<void(gldefs::GLhandle fbo), 2 * sizeof(void*)> callback;
  gl_depth_test_props depth_test;
  gl_stencil_test_props stencil_test;
  gl_blending_props blending;
  gl_culling_props culling;
  gl_pipeline::primitive_mode primitive;
  gl_pipeline::polygon_mode poly_mode;
  f32 poly_width;
  rectangle_pos<u32> viewport;
  rectangle_pos<u32> scissor;
};

class gl_extcmd_builder {
public:
  gl_extcmd_builder() noexcept;

public:
  gl_extcmd_builder& set_depth_test(const gl_depth_test_props& depth);
  gl_extcmd_builder& set_stencil_test(const gl_stencil_test_props& stencil);
  gl_extcmd_builder& set_blending(const gl_blending_props& blending);
  gl_extcmd_builder& set_culling(const gl_culling_props& culling);
  gl_extcmd_builder& set_primitive(gl_pipeline::primitive_mode primitive);
  gl_extcmd_builder& set_poly_mode(gl_pipeline::polygon_mode poly_mode);

  gl_extcmd_builder& set_viewport(const rectangle_pos<u32>& viewport);
  gl_extcmd_builder& set_viewport(u32 x, u32 y, u32 width, u32 height);
  gl_extcmd_builder& set_scissor(const rectangle_pos<u32>& scissor);
  gl_extcmd_builder& set_scissor(u32 x, u32 y, u32 width, u32 height);

  template<typename F>
  gl_extcmd_builder& set_callback(F&& func) {
    if (_callback.has_value()) {
      _callback.reset();
    }
    _callback.emplace(std::forward<F>(func));
    return *this;
  }

  template<typename F, typename... Args>
  gl_extcmd_builder& set_callback(std::in_place_type_t<F> tag, Args&&... args) {
    if (_callback.has_value()) {
      _callback.reset();
    }
    _callback.emplace(tag, std::forward<Args>(args)...);
    return *this;
  }

public:
  void reset();
  gl_ext_cmd build() const;

private:
  optional<inplace_trivial_fn<void(gldefs::GLhandle fbo), 2 * sizeof(void*)>> _callback;
  gl_stencil_test_props _stencil;
  gl_depth_test_props _depth;
  gl_blending_props _blending;
  gl_culling_props _culling;
  gl_pipeline::primitive_mode _primitive;
  gl_pipeline::polygon_mode _poly_mode;
  f32 _poly_width;
  rectangle_pos<u32> _viewport;
  optional<rectangle_pos<u32>> _scissor;
};

class gl_context {
private:
  struct create_t {};

  template<typename F>
  static constexpr bool _scope_frame_invocable =
    std::is_invocable_v<F, gl_context&> || std::is_invocable_v<F>;

  struct context_deleter {
    void operator()(gl_private* ptr) noexcept;
  };

  using context_data = std::unique_ptr<gl_private, context_deleter>;

public:
  struct gl_version {
    u32 major;
    u32 minor;
  };

public:
  explicit gl_context(create_t, context_data&& ctx) noexcept;

  explicit gl_context(const gl_surface_provider& surf_prov);

  template<gl_provider_type T>
  gl_context(T& surf_prov) : gl_context(::shogle::gl_surface_provider(surf_prov)) {}

public:
  static sv_expect<gl_context> create(const gl_surface_provider& surf_prov) noexcept;

  template<gl_provider_type T>
  static sv_expect<gl_context> create(T& surf_prov) noexcept {
    return ::shogle::gl_context::create(::shogle::gl_surface_provider(surf_prov));
  }

public:
  void start_frame(const gl_clear_opts& clear);
  void end_frame();

  void submit_immediate_command(const gl_draw_cmd& cmd,
                                ptr_view<const gl_framebuffer> target = {});
  void submit_immediate_command(const gl_ext_cmd& cmd, ptr_view<const gl_framebuffer> target = {});

  void submit_command(const gl_draw_cmd& cmd, ptr_view<const gl_framebuffer> target = {});
  void submit_command(const gl_ext_cmd& cmd, ptr_view<const gl_framebuffer> target = {});

  template<typename F>
  void scope_frame(const gl_clear_opts& clear, F&& scope)
  requires(_scope_frame_invocable<F>)
  {
    start_frame(clear);
    if constexpr (std::is_invocable_v<F, gl_context&>) {
      std::invoke(scope, *this);
    } else {
      std::invoke(scope);
    }
    end_frame();
  }

public:
  void destroy() noexcept;

public:
  gl_surface_provider provider() const;
  gldefs::GLenum get_error() const;
  gl_version version() const;
  std::string_view renderer_string() const;
  std::string_view vendor_string() const;
  std::string_view version_string() const;

private:
  context_data _ctx;

private:
  friend gl_private& impl::gl_get_private(gl_context& gl);
};

} // namespace shogle
