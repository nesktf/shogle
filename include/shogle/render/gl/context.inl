#define SHOGLE_RENDER_GL_CONTEXT_INL
#include <shogle/render/gl/context.hpp>
#undef SHOGLE_RENDER_GL_CONTEXT_INL

namespace shogle {

inline gl_frame_init_builder::gl_frame_init_builder() noexcept :
    _color(), _clear_flags(), _fbos() {}

inline gl_frame_init_builder& gl_frame_init_builder::set_clear_color(const color4& color) {
  return this->set_clear_color(color.r, color.g, color.b, color.a);
}

inline gl_frame_init_builder& gl_frame_init_builder::set_clear_color(f32 r, f32 g, f32 b, f32 a) {
  _color.r = r;
  _color.g = g;
  _color.b = b;
  _color.a = a;
  return *this;
}

inline gl_frame_init_builder& gl_frame_init_builder::set_clear_flag(gldefs::GLenum clear_flag) {
  _clear_flags = clear_flag;
  return *this;
}

inline gl_frame_init_builder& gl_frame_init_builder::clear_color() {
  _clear_flags |= (gldefs::GLenum)gl_clear_opt::CLEAR_COLOR;
  return *this;
}

inline gl_frame_init_builder& gl_frame_init_builder::clear_depth() {
  _clear_flags |= (gldefs::GLenum)gl_clear_opt::CLEAR_DEPTH;
  return *this;
}

inline gl_frame_init_builder& gl_frame_init_builder::clear_stencil() {
  _clear_flags |= (gldefs::GLenum)gl_clear_opt::CLEAR_STENCIL;
  return *this;
}

inline gl_frame_init_builder& gl_frame_init_builder::add_framebuffer(const gl_framebuffer& fbo,
                                                                     gldefs::GLenum clear_flag,
                                                                     const color4& color) {
  return this->add_framebuffer(fbo, clear_flag, color.r, color.g, color.b, color.a);
}

inline gl_frame_init_builder& gl_frame_init_builder::add_framebuffer(const gl_framebuffer& fbo,
                                                                     gldefs::GLenum clear_flags,
                                                                     f32 r, f32 g, f32 b, f32 a) {
  gl_clear_opt opt{.color = {r, g, b, a}, .clear_flags = (gl_clear_opt::clear_flag)clear_flags};
  _fbos.emplace_back(fbo, opt);
  return *this;
}

inline void gl_frame_init_builder::reset() {
  _color.r = 0.f;
  _color.g = 0.f;
  _color.b = 0.f;
  _color.a = 0.f;
  _clear_flags = 0;
  _fbos.clear();
}

inline gl_frame_initializer gl_frame_init_builder::build() const {
  return {
    .clear_opt =
      {
        .color = _color,
        .clear_flags = (gl_clear_opt::clear_flag)_clear_flags,
      },
    .fbos = {_fbos.data(), _fbos.size()},
  };
}

namespace impl {

template<typename Derived>
gl_basic_command_builder<Derived>::gl_basic_command_builder(
  const gl_vertex_layout& layout, const gl_graphics_pipeline& pipeline) noexcept :
    _vertex_layout(layout), _pipeline(pipeline), _shader_binds(), _vertex_buffers(), _unifs(),
    _textures(), _viewport(), _scissor(), _instances(1), _vertex_offset(), _vertex_count() {}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_vertex_layout(const gl_vertex_layout& layout) {
  _vertex_layout = layout;
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_pipeline(const gl_graphics_pipeline& pipeline) {
  _pipeline = pipeline;
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_viewport(const rectangle_pos<u32>& viewport) {
  return this->set_viewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_viewport(u32 x, u32 y, u32 width, u32 height) {
  _viewport.x = x;
  _viewport.y = y;
  _viewport.width = width;
  _viewport.height = height;
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_scissor(const rectangle_pos<u32>& viewport) {
  return this->set_viewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_scissor(u32 x, u32 y, u32 width, u32 height) {
  if (_scissor.has_value()) {
    _scissor->x = x;
    _scissor->y = y;
    _scissor->width = width;
    _scissor->height = height;
  } else {
    _scissor.emplace(x, y, width, height);
  }
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_instances(u32 instances) {
  _instances = std::max(1u, instances);
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_vertex_offset(u32 offset) {
  _vertex_offset = offset;
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::set_vertex_count(u32 count) {
  _vertex_count = count;
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::add_vertex_buffer(u32 location,
                                                              const gl_buffer& buffer) {
  _vertex_buffers.emplace_back(buffer, location);
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::add_shader_buffer(u32 location,
                                                              const gl_buffer& buffer, size_t size,
                                                              size_t offset) {
  _shader_binds.emplace_back(buffer, location, offset, size);
  return static_cast<Derived&>(*this);
}

template<typename Derived>
Derived& gl_basic_command_builder<Derived>::add_texture(u32 index, const gl_texture& texture) {
  _textures.emplace_back(texture, index);
  return static_cast<Derived&>(*this);
}

template<typename Derived>
template<::shogle::meta::attribute_type T>
Derived& gl_basic_command_builder<Derived>::add_uniform(u32 location, const T& value) {
  _unifs.emplace_back(location, value);
  return static_cast<Derived&>(*this);
}

template<typename Derived>
void gl_basic_command_builder<Derived>::_reset() {
  _shader_binds.clear();
  _vertex_buffers.clear();
  _unifs.clear();
  _textures.clear();
  _viewport.x = 0;
  _viewport.y = 0;
  _viewport.width = 0;
  _viewport.height = 0;
  _scissor.reset();
  _instances = 1;
  _vertex_offset = 0;
  _vertex_count = 0;
}

} // namespace impl

inline gl_indexed_command_builder::gl_indexed_command_builder(
  const gl_vertex_layout& layout, const gl_graphics_pipeline& pipeline,
  const gl_buffer& index_buffer, gl_indexed_command::index_format format) noexcept :
    impl::gl_basic_command_builder<gl_indexed_command_builder>(layout, pipeline),
    _index_buffer(index_buffer), _index_format(format), _index_count(0u) {}

inline gl_indexed_command_builder&
gl_indexed_command_builder::set_index_format(gl_indexed_command::index_format format) {
  _index_format = format;
  return *this;
}

inline gl_indexed_command_builder& gl_indexed_command_builder::set_index_count(u32 index_count) {
  _index_count = index_count;
  return *this;
}

inline void gl_indexed_command_builder::reset() {
  impl::gl_basic_command_builder<gl_indexed_command_builder>::_reset();
  _index_count = 0;
}

inline gl_indexed_command gl_indexed_command_builder::build() const {
  return {
    .vertex_layout = this->_vertex_layout,
    .pipeline = this->_pipeline,
    .index_buffer = _index_buffer,
    .vertex_buffers = {this->_vertex_buffers.data(), this->_vertex_buffers.size()},
    .shader_buffers = {this->_shader_binds.data(), this->_shader_binds.size()},
    .textures = {this->_textures.data(), this->_textures.size()},
    .uniforms = {this->_unifs.data(), this->_unifs.size()},
    .viewport = this->_viewport,
    .scissor = this->_scissor.has_value() ? *this->_scissor : this->_viewport,
    .vertex_offset = this->_vertex_offset,
    .vertex_count = this->_vertex_count,
    .index_count = _index_count,
    .format = _index_format,
    .instances = this->_instances,
  };
}

inline void gl_array_command_builder::reset() {
  impl::gl_basic_command_builder<gl_array_command_builder>::_reset();
}

inline gl_array_command gl_array_command_builder::build() const {
  return {
    .vertex_layout = this->_vertex_layout,
    .pipeline = this->_pipeline,
    .vertex_buffers = {this->_vertex_buffers.data(), this->_vertex_buffers.size()},
    .shader_buffers = {this->_shader_binds.data(), this->_shader_binds.size()},
    .textures = {this->_textures.data(), this->_textures.size()},
    .uniforms = {this->_unifs.data(), this->_unifs.size()},
    .viewport = this->_viewport,
    .scissor = this->_scissor.has_value() ? *this->_scissor : this->_viewport,
    .vertex_offset = this->_vertex_offset,
    .vertex_count = this->_vertex_count,
    .instances = this->_instances,
  };
}

inline gl_external_command_builder::gl_external_command_builder(
  gl_external_command::callback_type callback) :
    _callback(callback), _stencil(::shogle::gl_stencil_test_props::make_default(false)),
    _depth(::shogle::gl_depth_test_props::make_default(false)),
    _blending(::shogle::gl_blending_props::make_default(false)),
    _culling(::shogle::gl_culling_props::make_default(false)),
    _primitive(gl_graphics_pipeline::PRIMITIVE_TRIANGLES),
    _poly_mode(gl_graphics_pipeline::POLY_MODE_FILL), _poly_width(1.f), _viewport(), _scissor() {}

inline gl_external_command_builder&
gl_external_command_builder::set_callback(gl_external_command::callback_type callback) {
  _callback = callback;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_depth_test(const gl_depth_test_props& depth) {
  _depth = depth;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_stencil_test(const gl_stencil_test_props& stencil) {
  _stencil = stencil;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_blending(const gl_blending_props& blending) {
  _blending = blending;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_culling(const gl_culling_props& culling) {
  _culling = culling;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_primitive(gl_graphics_pipeline::primitive_mode primitive) {
  _primitive = primitive;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_poly_mode(gl_graphics_pipeline::polygon_mode poly_mode) {
  _poly_mode = poly_mode;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_viewport(const rectangle_pos<u32>& viewport) {
  return this->set_viewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

inline gl_external_command_builder&
gl_external_command_builder::set_viewport(u32 x, u32 y, u32 width, u32 height) {
  _viewport.x = x;
  _viewport.y = y;
  _viewport.width = width;
  _viewport.height = height;
  return *this;
}

inline gl_external_command_builder&
gl_external_command_builder::set_scissor(const rectangle_pos<u32>& scissor) {
  return this->set_scissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

inline gl_external_command_builder&
gl_external_command_builder::set_scissor(u32 x, u32 y, u32 width, u32 height) {
  if (_scissor.has_value()) {
    _scissor->x = x;
    _scissor->y = y;
    _scissor->width = width;
    _scissor->height = height;
  } else {
    _scissor.emplace(x, y, width, height);
  }
  return *this;
}

inline void gl_external_command_builder::reset() {
  _stencil = gl_stencil_test_props::make_default(false);
  _depth = gl_depth_test_props::make_default(false);
  _blending = gl_blending_props::make_default(false);
  _culling = gl_culling_props::make_default(false);
  _primitive = gl_graphics_pipeline::PRIMITIVE_TRIANGLES;
  _poly_mode = gl_graphics_pipeline::POLY_MODE_FILL;
  _poly_width = 1.f;
  _viewport.x = 0;
  _viewport.y = 0;
  _viewport.width = 0;
  _viewport.height = 0;
  _scissor.reset();
}

inline gl_external_command gl_external_command_builder::build() const {
  return {
    .callback = _callback,
    .depth_test = _depth,
    .stencil_test = _stencil,
    .blending = _blending,
    .culling = _culling,
    .primitive = _primitive,
    .poly_mode = _poly_mode,
    .poly_width = _poly_width,
    .viewport = _viewport,
    .scissor = this->_scissor.has_value() ? *this->_scissor : this->_viewport,
  };
}

template<typename F>
void gl_context::scope_frame(const gl_frame_initializer& init, F&& scope)
requires(_scope_frame_invocable<F>)
{
  start_frame(init);
  if constexpr (std::is_invocable_v<F, gl_context&>) {
    std::invoke(scope, *this);
  } else {
    std::invoke(scope);
  }
  end_frame();
}

} // namespace shogle
