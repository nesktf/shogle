#pragma once

#include <shogle/render/gl/common.hpp>
#include <shogle/render/gl/pipeline.hpp>

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
  gl_clear_builder& set_viewport(const rectangle_pos<u32>& viewport);
  gl_clear_builder& set_viewport(u32 x, u32 y, u32 width, u32 height);
  gl_clear_builder& set_clear_color(const color4& color);
  gl_clear_builder& set_clear_color(f32 r, f32 g, f32 b, f32 a = 1.f);
  gl_clear_builder& set_clear_flag(gl_clear_opts::clear_flag flag);

  gl_clear_builder& add_framebuffer(const gl_framebuffer& fbo, const rectangle_pos<u32>& viewport,
                                    gldefs::GLenum clear_flags, const color4& color);
  gl_clear_builder& add_framebuffer(const gl_framebuffer& fbo, const rectangle_pos<u32>& viewport,
                                    gldefs::GLenum clear_flags, f32 r, f32 g, f32 b, f32 a = 1.f);

public:
  void reset();
  gl_clear_opts build() const;

private:
  color4 _color;
  optional<rectangle_pos<u32>> _viewport;
  gldefs::GLenum _clear_flags;
  std::vector<gl_clear_opts::fbo_initializer> _fbos;
};

struct gl_draw_command {
public:
  enum index_format : gldefs::GLenum {
    INDEX_FORMAT_I8 = 0, // GL_BYTE
    INDEX_FORMAT_U8,     // GL_UNSIGNED_BYTE
    INDEX_FORMAT_I16,    // GL_SHORT
    INDEX_FORMAT_U16,    // GL_UNSIGNED_SHORT
    INDEX_FORMAT_I32,    // GL_INT
    INDEX_FORMAT_U32,    // GL_UNSIGNED_INT
  };

  struct index_binding {
    gldefs::GLhandle buffer;
    index_format format;
    size_t index_offset;
  };

  struct texture_binding {
    gldefs::GLhandle texture;
    gldefs::GLenum type;
    u32 index;
  };

  struct vertex_binding {
    gldefs::GLhandle buffer;
    u32 location;
  };

  struct shader_binding {
    gldefs::GLhandle buffer;
    gldefs::GLenum type;
    size_t size;
    size_t offset;
    u32 location;
  };

  struct push_uniform {
    template<::shogle::meta::attribute_type T>
    push_uniform(u32 location_, const T& data_) :
        type(meta::attribute_traits<T>::tag), location(location_) {
      std::memcpy(&data[0], &data_, sizeof(T));
    }

    template<typename T>
    requires(std::is_trivially_copyable_v<T>)
    push_uniform(u32 location_, const T& data_, attribute_type type_) :
        type(type_), location(location_) {
      std::memcpy(&data[0], &data_, sizeof(T));
    }

    alignas(::shogle::mat4) u8 data[sizeof(::shogle::mat4)];
    attribute_type type;
    u32 location;
  };

public:
  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_graphics_pipeline> pipeline;
  span<const vertex_binding> vertex_bindings;
  span<const shader_binding> shader_bindings;
  span<const texture_binding> texture_bindings;
  span<const push_uniform> uniforms;
  optional<index_binding> index_bind;
  optional<rectangle_pos<u32>> viewport;
  optional<rectangle_pos<u32>> scissor;
  size_t vertex_offset;
  u32 draw_count;
  u32 instances;
};

class gl_command_builder {
public:
  gl_command_builder() noexcept;

public:
  gl_command_builder& set_vertex_layout(const gl_vertex_layout& layout);
  gl_command_builder& set_pipeline(const gl_graphics_pipeline& pipeline);

  gl_command_builder& set_viewport(const rectangle_pos<u32>& viewport);
  gl_command_builder& set_viewport(u32 x, u32 y, u32 width, u32 height);
  gl_command_builder& set_scissor(const rectangle_pos<u32>& scissor);
  gl_command_builder& set_scissor(u32 x, u32 y, u32 width, u32 height);

  gl_command_builder& set_instances(u32 instances);
  gl_command_builder& set_vertex_offset(size_t offset);
  gl_command_builder& set_draw_count(u32 count);
  gl_command_builder& set_index_buffer(const gl_buffer& buffer,
                                       gl_draw_command::index_format format,
                                       size_t index_offset = 0);

  gl_command_builder& add_vertex_buffer(const gl_buffer& buffer, u32 location = 0);
  gl_command_builder& add_shader_buffer(u32 location, const gl_buffer& buffer, size_t size = 0,
                                        size_t offset = 0);
  gl_command_builder& add_texture(const gl_texture& texture, u32 location);

  template<::shogle::meta::attribute_type T>
  gl_command_builder& add_uniform(const T& value, u32 location) {
    _uniforms.emplace_back(location, value);
    return *this;
  }

  template<typename T>
  requires(std::is_trivially_copyable_v<T>)
  gl_command_builder& add_uniform(const T& value, u32 location, attribute_type tag) {
    _uniforms.emplace_back(location, value, tag);
    return *this;
  }

public:
  void reset();
  gl_draw_command build() const;

private:
  ptr_view<const gl_vertex_layout> _vertex_layout;
  ptr_view<const gl_graphics_pipeline> _pipeline;
  std::vector<gl_draw_command::vertex_binding> _vertex_binds;
  std::vector<gl_draw_command::shader_binding> _shader_binds;
  std::vector<gl_draw_command::texture_binding> _texture_binds;
  std::vector<gl_draw_command::push_uniform> _uniforms;
  optional<gl_draw_command::index_binding> _index;
  optional<rectangle_pos<u32>> _viewport;
  optional<rectangle_pos<u32>> _scissor;
  size_t _vertex_offset;
  u32 _draw_count;
  u32 _instances;
};

struct gl_external_command {
public:
  using callback_type = function_view<void(gl_context& gl, gldefs::GLhandle fbo)>;

public:
  callback_type callback;
  gl_depth_test_props depth_test;
  gl_stencil_test_props stencil_test;
  gl_blending_props blending;
  gl_culling_props culling;
  gl_graphics_pipeline::primitive_mode primitive;
  gl_graphics_pipeline::polygon_mode poly_mode;
  f32 poly_width;
  rectangle_pos<u32> viewport;
  rectangle_pos<u32> scissor;
};

class gl_external_command_builder {
public:
  gl_external_command_builder() noexcept;

public:
  gl_external_command_builder& set_callback(gl_external_command::callback_type callback);

  gl_external_command_builder& set_depth_test(const gl_depth_test_props& depth);
  gl_external_command_builder& set_stencil_test(const gl_stencil_test_props& stencil);
  gl_external_command_builder& set_blending(const gl_blending_props& blending);
  gl_external_command_builder& set_culling(const gl_culling_props& culling);
  gl_external_command_builder& set_primitive(gl_graphics_pipeline::primitive_mode primitive);
  gl_external_command_builder& set_poly_mode(gl_graphics_pipeline::polygon_mode poly_mode);

  gl_external_command_builder& set_viewport(const rectangle_pos<u32>& viewport);
  gl_external_command_builder& set_viewport(u32 x, u32 y, u32 width, u32 height);
  gl_external_command_builder& set_scissor(const rectangle_pos<u32>& scissor);
  gl_external_command_builder& set_scissor(u32 x, u32 y, u32 width, u32 height);

public:
  void reset();
  gl_external_command build() const;

private:
  gl_external_command::callback_type _callback;
  gl_stencil_test_props _stencil;
  gl_depth_test_props _depth;
  gl_blending_props _blending;
  gl_culling_props _culling;
  gl_graphics_pipeline::primitive_mode _primitive;
  gl_graphics_pipeline::polygon_mode _poly_mode;
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

  gl_context(gl_surface_provider& surf_prov);

public:
  static sv_expect<gl_context> create(gl_surface_provider& surf_prov) noexcept;

public:
  void start_frame(const gl_clear_opts& clear);
  void submit_command(const gl_draw_command& cmd, ptr_view<const gl_framebuffer> target = {});
  void submit_command(const gl_external_command& cmd, ptr_view<const gl_framebuffer> target = {});
  void end_frame();

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
  gl_surface_provider& provider() const;
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
