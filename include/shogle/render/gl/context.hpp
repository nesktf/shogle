#pragma once

#include <shogle/render/gl/common.hpp>
#include <shogle/render/gl/pipeline.hpp>

namespace shogle {

struct gl_texture_binding {
  ref_view<const gl_texture> texture;
  u32 index;
};

struct gl_push_uniform {
  template<meta::attribute_type T>
  gl_push_uniform(u32 location_, const T& data_) :
      data(std::in_place_type_t<T>{}, data_), type(meta::attribute_traits<T>::tag),
      location(location_) {}

  ntf::inplace_trivial<sizeof(mat4), alignof(mat4)> data;
  attribute_type type;
  u32 location;
};

struct gl_buffer_binding {
  ref_view<const gl_buffer> buffer;
  u32 location;
};

struct gl_shader_binding {
  ref_view<const gl_buffer> buffer;
  u32 location;
  size_t offset;
  size_t size;
};

struct gl_clear_opt {
public:
  enum clear_flag : gldefs::GLenum {
    CLEAR_NONE = 0x00000000,    // GL_NONE
    CLEAR_COLOR = 0x00004000,   // GL_COLOR_BUFFER_BIT
    CLEAR_DEPTH = 0x00000100,   // GL_DEPTH_BUFFER_BIT
    CLEAR_STENCIL = 0x00000400, // GL_STENCIL_BUFFER_BIT
  };

public:
  color4 color;
  clear_flag clear_flags;
};

struct gl_frame_initializer {
public:
  struct fbo_initializer {
    ref_view<const gl_framebuffer> fbo;
    gl_clear_opt clear_opts;
  };

public:
  gl_clear_opt clear_opt;
  span<const fbo_initializer> fbos;
};

class gl_frame_init_builder {
public:
  gl_frame_init_builder() noexcept;

public:
  gl_frame_init_builder& set_clear_color(const color4& color);
  gl_frame_init_builder& set_clear_color(f32 r, f32 g, f32 b, f32 a = 1.f);

  gl_frame_init_builder& set_clear_flag(gldefs::GLenum clear_flag);
  gl_frame_init_builder& clear_color();
  gl_frame_init_builder& clear_depth();
  gl_frame_init_builder& clear_stencil();

  gl_frame_init_builder& add_framebuffer(const gl_framebuffer& fbo, gldefs::GLenum clear_flags,
                                         const color4& color);
  gl_frame_init_builder& add_framebuffer(const gl_framebuffer& fbo, gldefs::GLenum clear_flags,
                                         f32 r, f32 g, f32 b, f32 a = 1.f);

public:
  void reset();
  gl_frame_initializer build() const;

private:
  color4 _color;
  gldefs::GLenum _clear_flags;
  std::vector<gl_frame_initializer::fbo_initializer> _fbos;
};

namespace impl {

template<typename Derived>
class gl_basic_command_builder {
public:
  gl_basic_command_builder(const gl_vertex_layout& layout,
                           const gl_graphics_pipeline& pipeline) noexcept;

public:
  Derived& set_vertex_layout(const gl_vertex_layout& layout);
  Derived& set_pipeline(const gl_graphics_pipeline& pipeline);
  Derived& set_viewport(const rectangle_pos<u32>& viewport);
  Derived& set_viewport(u32 x, u32 y, u32 width, u32 height);
  Derived& set_scissor(const rectangle_pos<u32>& viewport);
  Derived& set_scissor(u32 x, u32 y, u32 width, u32 height);
  Derived& set_instances(u32 instances);

  Derived& set_vertex_offset(u32 offset);
  Derived& set_vertex_count(u32 count);

  Derived& add_vertex_buffer(u32 location, const gl_buffer& buffer);
  Derived& add_shader_buffer(u32 location, const gl_buffer& buffer, size_t size = 0,
                             size_t offset = 0);
  Derived& add_texture(u32 index, const gl_texture& texture);

  template<::shogle::meta::attribute_type T>
  Derived& add_uniform(u32 location, const T& value);

protected:
  void _reset();

protected:
  ref_view<const gl_vertex_layout> _vertex_layout;
  ref_view<const gl_graphics_pipeline> _pipeline;
  std::vector<gl_shader_binding> _shader_binds;
  std::vector<gl_buffer_binding> _vertex_buffers;
  std::vector<gl_push_uniform> _unifs;
  std::vector<gl_texture_binding> _textures;
  rectangle_pos<u32> _viewport;
  ntf::optional<rectangle_pos<u32>> _scissor;
  u32 _instances;
  u32 _vertex_offset;
  u32 _vertex_count;
};

} // namespace impl

struct gl_indexed_cmd {
public:
  enum index_format : gldefs::GLenum {
    INDEX_FORMAT_I8 = 0x1400,  // GL_BYTE
    INDEX_FORMAT_U8 = 0x1401,  // GL_UNSIGNED_BYTE
    INDEX_FORMAT_I16 = 0x402,  // GL_SHORT
    INDEX_FORMAT_U16 = 0x1403, // GL_UNSIGNED_SHORT
    INDEX_FORMAT_I32 = 0x1404, // GL_INT
    INDEX_FORMAT_U32 = 0x1405, // GL_UNSIGNED_INT
  };

public:
  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_graphics_pipeline> pipeline;
  ref_view<const gl_buffer> index_buffer;
  span<const gl_buffer_binding> vertex_buffers;
  span<const gl_shader_binding> shader_buffers;
  span<const gl_texture_binding> textures;
  span<const gl_push_uniform> uniforms;
  rectangle_pos<u32> viewport;
  rectangle_pos<u32> scissor;
  u32 vertex_offset;
  u32 vertex_count;
  u32 index_count;
  index_format format;
  u32 instances;
};

class gl_indexed_command_builder :
    public impl::gl_basic_command_builder<gl_indexed_command_builder> {
public:
  gl_indexed_command_builder(const gl_vertex_layout& layout, const gl_graphics_pipeline& pipeline,
                             const gl_buffer& index_buffer,
                             const gl_indexed_cmd::index_format format) noexcept;

public:
  gl_indexed_command_builder& set_index_format(gl_indexed_cmd::index_format format);
  gl_indexed_command_builder& set_index_count(u32 index_count);

public:
  void reset();
  gl_indexed_cmd build() const;

private:
  ref_view<const gl_buffer> _index_buffer;
  gl_indexed_cmd::index_format _index_format;
  u32 _index_count;
};

struct gl_array_cmd {
  ref_view<const gl_vertex_layout> vertex_layout;
  ref_view<const gl_graphics_pipeline> pipeline;
  span<const gl_buffer_binding> vertex_buffers;
  span<const gl_shader_binding> shader_buffers;
  span<const gl_texture_binding> textures;
  span<const gl_push_uniform> uniforms;
  rectangle_pos<u32> viewport;
  rectangle_pos<u32> scissor;
  u32 vertex_offset;
  u32 vertex_count;
  u32 instances;
};

class gl_array_command_builder : public impl::gl_basic_command_builder<gl_array_command_builder> {
public:
  using impl::gl_basic_command_builder<gl_array_command_builder>::gl_basic_command_builder;

public:
  void reset();
  gl_array_cmd build() const;
};

struct gl_external_cmd {
public:
  using callback_type = ntf::function_view<void(gl_context& gl, gldefs::GLhandle fbo)>;

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
  gl_external_command_builder(gl_external_cmd::callback_type callback);

public:
  gl_external_command_builder& set_callback(gl_external_cmd::callback_type callback);

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
  gl_external_cmd build() const;

private:
  gl_external_cmd::callback_type _callback;
  gl_stencil_test_props _stencil;
  gl_depth_test_props _depth;
  gl_blending_props _blending;
  gl_culling_props _culling;
  gl_graphics_pipeline::primitive_mode _primitive;
  gl_graphics_pipeline::polygon_mode _poly_mode;
  f32 _poly_width;
  rectangle_pos<u32> _viewport;
  ntf::optional<rectangle_pos<u32>> _scissor;
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
  void start_frame(const gl_frame_initializer& init);
  gl_sv_expect<void> submit_indexed_draw_command(const gl_indexed_cmd& cmd,
                                                 ptr_view<const gl_framebuffer> target = {});
  gl_sv_expect<void> submit_draw_command(const gl_array_cmd& cmd,
                                         ptr_view<const gl_framebuffer> target = {});
  void submit_external_command(const gl_external_cmd& cmd,
                               ptr_view<const gl_framebuffer> target = {});
  void end_frame();

  template<typename F>
  void scope_frame(const gl_frame_initializer& init, F&& scope)
  requires(_scope_frame_invocable<F>);

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

#ifndef SHOGLE_RENDER_GL_CONTEXT_INL
#include <shogle/render/gl/context.inl>
#endif
