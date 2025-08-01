#pragma once

#include <shogle/render.hpp>
#include <imgui.h>

namespace ntf::impl {

template<typename Derived>
class rcontext_ops {
  ntfr::context_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::context_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid context handle");
    return ptr;
  }

public:
  operator ntfr::context_t() const noexcept { return _ptr(); }

  void start_frame() const {
    ntfr::start_frame(_ptr());
  }
  void end_frame() const {
    ntfr::end_frame(_ptr());
  }
  void device_wait() const {
    ntfr::device_wait(_ptr());
  }
  void submit_render_command(const ntfr::render_cmd& cmd) const {
    ntfr::submit_render_command(_ptr(), cmd);
  }
  void submit_external_command(const ntfr::external_cmd& cmd) const {
    ntfr::submit_external_command(_ptr(), cmd);
  }

  ntfr::context_api api() const {
    return ntfr::get_api(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class context_view : public impl::rcontext_ops<context_view> {
public:
  context_view(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  context_t get() const noexcept { return _ctx; }

private:
  context_t _ctx;
};

class context : public ::ntf::impl::rcontext_ops<context> {
private:
  struct deleter_t {
    void operator()(context_t ctx) noexcept {
      ntfr::destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<context_t>, deleter_t>;

public:
  explicit context(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  static expect<context> create(const context_params& params) {
    return ntfr::create_context(params)
    .transform([](context_t ctx) -> context {
      return context{ctx};
    });
  }

public:
  context_t get() const noexcept { return _ctx.get(); }

public:
  operator context_view() const noexcept { return {get()}; }

private:
  uptr_type _ctx;
};

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rbuffer_ops {
  ntfr::buffer_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::buffer_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid buffer handle");
    return ptr;
  }

public:
  operator ntfr::buffer_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  template<typename T>
  requires(std::is_trivially_copyable_v<T>)
  ntfr::expect<void> upload(const T& data, size_t offset = 0u) const {
    return ntfr::buffer_upload(_ptr(), sizeof(T), offset, std::addressof(data));
  }

  ntfr::expect<void> upload(const ntfr::buffer_data& data) const {
    return ntfr::buffer_upload(_ptr(), data);
  }
  ntfr::expect<void> upload(size_t size, size_t offset, const void* data) const {
    return ntfr::buffer_upload(_ptr(), size, offset, data);
  }
  ntfr::expect<void*> map(size_t size, size_t offset) const {
    return ntfr::buffer_map(_ptr(), size, offset);
  }
  void unmap(void* mapped) const {
    ntfr::buffer_unmap(_ptr(), mapped);
  }

  ntfr::context_view context() const {
    return {ntfr::buffer_get_ctx(_ptr())};
  }
  ntfr::buffer_type type() const {
    return ntfr::buffer_get_type(_ptr());
  }
  size_t size() const {
    return ntfr::buffer_get_size(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::buffer_get_id(_ptr());
  }
};

template<typename Derived>
class rbuffer_view : public rbuffer_ops<Derived> {
protected:
  rbuffer_view(ntfr::buffer_t buff) noexcept :
    _buff{buff} {}

public:
  ntfr::buffer_t get() const noexcept { return _buff; }

  bool empty() const noexcept { return _buff == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::buffer_t _buff;
};

template<typename Derived>
class rbuffer_owning : public rbuffer_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::buffer_t buff) noexcept {
      ntfr::destroy_buffer(buff);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::buffer_t>, deleter_t>;

protected:
  rbuffer_owning(ntfr::buffer_t buff) noexcept :
    _buff{buff} {}

public:
  ntfr::buffer_t get() const noexcept { return _buff.get(); }
  [[nodiscard]] ntfr::buffer_t release() noexcept { return _buff.release(); }

  bool empty() const noexcept { return _buff.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _buff;
};

} // namespace ntf::impl

namespace ntf::render {

class buffer_view : public ntf::impl::rbuffer_view<buffer_view> {
public:
  buffer_view() noexcept :
    ntf::impl::rbuffer_view<buffer_view>{nullptr} {}

  buffer_view(buffer_t buff) noexcept :
    ntf::impl::rbuffer_view<buffer_view>{buff} {}
};

class buffer : public ntf::impl::rbuffer_owning<buffer> {
public:
  buffer() noexcept :
    ntf::impl::rbuffer_owning<buffer>{nullptr} {}

  explicit buffer(buffer_t buff) noexcept :
    ntf::impl::rbuffer_owning<buffer>{buff} {}

public:
  static expect<buffer> create(context_view ctx, const buffer_desc& desc) {
    return ntfr::create_buffer(ctx.get(), desc)
    .transform([](buffer_t buff) -> buffer {
      buffer d{buff};
      return buffer{buff};
    });
  }

  template<typename U, size_t N>
  static expect<buffer> create(context_view ctx, U (&arr)[N],
                               buffer_flag flags, buffer_type type, size_t offset = 0u) {
    const buffer_data data {
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = offset,
    };
    return create(ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = data,
    });
  }

public:
  operator buffer_view() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
class typed_buffer;

template<buffer_type buff_enum>
class typed_buffer_view : public ntf::impl::rbuffer_view<typed_buffer_view<buff_enum>> {
private:
  friend typed_buffer<buff_enum>;

public:
  template<buffer_type _buff_enum>
  friend typed_buffer_view<_buff_enum> to_typed(buffer_view buff) noexcept;

public:
  typed_buffer_view() noexcept :
    ntf::impl::rbuffer_view<typed_buffer_view<buff_enum>>{nullptr} {}

private:
  typed_buffer_view(buffer_t buff) noexcept :
    ntf::impl::rbuffer_view<typed_buffer_view<buff_enum>>{buff} {}

public:
  operator buffer_view() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
typed_buffer_view<buff_enum> to_typed(buffer_view buff) noexcept {
  buffer_t ptr = nullptr;
  if (buff.type() == buff_enum) {
    ptr = buff.get();
  }
  return typed_buffer_view<buff_enum>{ptr};
}

template<buffer_type buff_enum>
class typed_buffer : public ntf::impl::rbuffer_owning<typed_buffer<buff_enum>> {
public:
  template<buffer_type _buff_enum>
  friend typed_buffer<_buff_enum> to_typed(buffer&& buff) noexcept;

public:
  typed_buffer() noexcept :
    ntf::impl::rbuffer_owning<typed_buffer<buff_enum>>{nullptr} {}

private:
  typed_buffer(buffer_t buff) noexcept :
    ntf::impl::rbuffer_owning<typed_buffer<buff_enum>>{buff} {}

public:
  static expect<typed_buffer> create(context_view ctx, const typed_buffer_desc& desc) {
    return ntfr::create_buffer(ctx.get(), {
      .type = buff_enum,
      .flags = desc.flags,
      .size = desc.size,
      .data = desc.data,
    })
    .transform([](buffer_t buff) -> typed_buffer {
      return typed_buffer{buff};
    });
  }

  template<typename U, size_t N>
  static expect<typed_buffer> create(context_view ctx, U (&arr)[N], buffer_flag flags,
                                     size_t offset = 0u) {
    const buffer_data data {
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = offset,
    };
    return create(ctx, {
      .flags = flags,
      .size = sizeof(arr),
      .data = data,
    });
  }

public:
  operator buffer_view() const noexcept { return {this->get()}; }
  operator typed_buffer_view<buff_enum>() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
typed_buffer<buff_enum> to_typed(buffer&& buff) noexcept {
  buffer_t ptr = nullptr;
  if (buff.type() == buff_enum){
    ptr = buff.release();
  }
  return typed_buffer<buff_enum>{ptr};
}

using vertex_buffer = typed_buffer<buffer_type::vertex>;
using vertex_buffer_view = typed_buffer_view<buffer_type::vertex>;

using index_buffer = typed_buffer<buffer_type::index>;
using index_buffer_view = typed_buffer_view<buffer_type::index>;

using shader_storage_buffer = typed_buffer<buffer_type::shader_storage>;
using shader_storage_buffer_view = typed_buffer_view<buffer_type::shader_storage>;

using uniform_buffer = typed_buffer<buffer_type::uniform>;
using uniform_buffer_view = typed_buffer_view<buffer_type::uniform>;

using texel_buffer = typed_buffer<buffer_type::texel>;
using texel_buffer_view = typed_buffer_view<buffer_type::texel>;

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rframebuffer_ops {
  ntfr::framebuffer_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::framebuffer_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid frambuffer handle");
    return ptr;
  }

public:
  operator ntfr::framebuffer_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  void clear_flags(ntfr::clear_flag flags) const {
    ntfr::framebuffer_set_clear_flags(_ptr(), flags);
  }
  void viewport(const uvec4& vp) const {
    ntfr::framebuffer_set_viewport(_ptr(), vp);
  }
  void clear_color(const ntfr::color4& color) const {
    ntfr::framebuffer_set_clear_color(_ptr(), color);
  }

  ntfr::context_view context() const {
    return {ntfr::framebuffer_get_ctx(_ptr())};
  }
  ntfr::clear_flag clear_flags() const {
    return ntfr::framebuffer_get_clear_flags(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::framebuffer_get_id(_ptr());
  }
  uvec4 viewport() const {
    return ntfr::framebuffer_get_viewport(_ptr());
  }
  ntfr::color4 clear_color() const {
    return ntfr::framebuffer_get_clear_color(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class framebuffer_view : public impl::rframebuffer_ops<framebuffer_view> {
public:
  framebuffer_view() noexcept :
    _pip{nullptr} {}

  framebuffer_view(framebuffer_t pip) noexcept :
    _pip{pip} {}

public:
  framebuffer_t get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  framebuffer_t _pip;
};

class framebuffer : public impl::rframebuffer_ops<framebuffer> {
private:
  struct deleter_t {
    void operator()(framebuffer_t pip) noexcept {
      ntfr::destroy_framebuffer(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::framebuffer_t>, deleter_t>;

public:
  static framebuffer_view get_default(context_view ctx) {
    return {ntfr::get_default_framebuffer(ctx.get())};
  }

public:
  framebuffer() noexcept :
    _pip{nullptr} {}

  explicit framebuffer(framebuffer_t pip) noexcept :
    _pip{pip} {}

public:
  static expect<framebuffer> create(context_view ctx, const fbo_image_desc& desc){
    return ntfr::create_framebuffer(ctx.get(), desc)
    .transform([](framebuffer_t pip) -> framebuffer {
      return framebuffer{pip};
    });
  }

public:
  framebuffer_t get() const noexcept { return _pip.get(); }
  [[nodiscard]] framebuffer_t release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf

namespace ntf::render {

class uniform_view {
public:
  constexpr uniform_view() noexcept :
    _unif{nullptr} {}

  constexpr uniform_view(uniform_t unif) noexcept :
    _unif{unif} {}

public:
  attribute_type type() const {
    return ntfr::uniform_get_type(_assert_get());
  }

  std::string_view name() const {
    return ntfr::uniform_get_name(_assert_get());
  }

  u32 location() const {
    return ntfr::uniform_get_location(_assert_get());
  }

private:
  ntfr::uniform_t _assert_get() const noexcept(NTF_ASSERT_NOEXCEPT) {
    NTF_ASSERT(_unif, "Invalid uniform handle");
    return _unif;
  }

public:
  operator ntfr::uniform_t() const { return _assert_get(); }

  ntfr::uniform_t get() const noexcept { return _unif; }

  bool empty() const noexcept {return _unif == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uniform_t _unif;
};

template<meta::attribute_type T>
uniform_const format_uniform_const(uniform_view uniform, const T& data) {
  return {
    .data = {std::in_place_type_t<T>{}, data},
    .type = meta::attribute_traits<T>::tag,
    .location = uniform.location(),
  };
}

template<meta::attribute_type T>
uniform_const format_uniform_const(u32 location, const T& data){
  return {
    .data = {std::in_place_type_t<T>{}, data},
    .type = meta::attribute_traits<T>::tag,
    .location = location,
  };
}

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rpipeline_ops {
  ntfr::pipeline_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::pipeline_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid pipeline handle");
    return ptr;
  }

public:
  operator ntfr::pipeline_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::context_view context() const {
    return {ntfr::pipeline_get_ctx(_ptr())};
  }
  ntfr::stages_flag stages() const {
    return ntfr::pipeline_get_stages(_ptr());
  }
  ntfr::uniform_view uniform(ntfr::cstring_view<char> name) const {
    return {ntfr::pipeline_get_uniform(_ptr(), name)};
  }
  size_t uniform_count() const {
    return ntfr::pipeline_get_uniform_count(_ptr());
  }
  cspan<ntfr::uniform_t> uniforms() const {
    return ntfr::pipeline_get_uniforms(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::pipeline_get_id(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class pipeline_view : public impl::rpipeline_ops<pipeline_view> {
public:
  pipeline_view() noexcept :
    _pip{nullptr} {}

  pipeline_view(pipeline_t pip) noexcept :
    _pip{pip} {}

public:
  pipeline_t get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  pipeline_t _pip;
};

class pipeline : public impl::rpipeline_ops<pipeline> {
private:
  struct deleter_t {
    void operator()(pipeline_t pip) noexcept {
      ntfr::destroy_pipeline(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::pipeline_t>, deleter_t>;

public:
  pipeline() noexcept :
    _pip{nullptr} {}

  explicit pipeline(pipeline_t pip) noexcept :
    _pip{pip} {}

public:
  static expect<pipeline> create(context_view ctx, const pipeline_desc& desc){
    return ntfr::create_pipeline(ctx.get(), desc)
    .transform([](pipeline_t pip) -> pipeline {
      return pipeline{pip};
    });
  }

public:
  pipeline_t get() const noexcept { return _pip.get(); }
  [[nodiscard]] pipeline_t release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf

namespace ntf::impl {

template<typename Derived>
class rshader_ops {
  ntfr::shader_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::shader_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid shader handle");
    return ptr;
  }

public:
  operator ntfr::shader_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::context_view context() const {
    return {ntfr::shader_get_ctx(_ptr())};
  }
  ntfr::shader_type type() const {
    return ntfr::shader_get_type(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::shader_get_id(_ptr());
  }
};

template<typename Derived>
class rshader_view : public rshader_ops<Derived> {
protected:
  rshader_view(ntfr::shader_t shad) noexcept :
    _shad{shad} {}

public:
  ntfr::shader_t get() const noexcept { return _shad; }

  bool empty() const noexcept { return _shad == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::shader_t _shad;
};

template<typename Derived>
class rshader_owning : public rshader_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::shader_t shad) noexcept {
      ntfr::destroy_shader(shad);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::shader_t>, deleter_t>;

protected:
  rshader_owning(ntfr::shader_t shad) noexcept :
    _shad{shad} {}

public:
  ntfr::shader_t get() const noexcept { return _shad.get(); }
  [[nodiscard]] ntfr::shader_t release() noexcept { return _shad.release(); }

  bool empty() const noexcept { return _shad.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _shad;
};

} // namespace ntf::impl

namespace ntf::render {

class shader_view : public impl::rshader_view<shader_view> {
public:
  shader_view() noexcept :
    impl::rshader_view<shader_view>{nullptr} {}

  shader_view(ntfr::shader_t shad) noexcept :
    impl::rshader_view<shader_view>{shad} {}
};

class shader : public impl::rshader_owning<shader> {
public:
  shader() noexcept :
    impl::rshader_owning<shader>{nullptr} {}

  explicit shader(ntfr::shader_t shad) noexcept :
    impl::rshader_owning<shader>{shad} {}
  
public:
  static expect<shader> create(context_view ctx, const shader_desc& desc) {
    return ntfr::create_shader(ctx.get(), desc)
    .transform([](shader_t shad) -> shader {
      return shader{shad};
    });
  }

public:
  operator shader_view() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
class typed_shader;

template<shader_type shad_enum>
class typed_shader_view : public impl::rshader_view<typed_shader_view<shad_enum>> {
private:
  friend typed_shader<shad_enum>;

public:
  template<shader_type _shad_enum>
  friend typed_shader_view<_shad_enum> to_typed(shader_view shad) noexcept;

public:
  typed_shader_view() noexcept :
    impl::rshader_view<typed_shader_view<shad_enum>>{nullptr} {}

private:
  typed_shader_view(shader_t shad) noexcept :
    impl::rshader_view<typed_shader_view<shad_enum>>{shad} {}

public:
  operator shader_view() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
typed_shader_view<shad_enum> to_typed(shader_view shad) noexcept {
  shader_t ptr = nullptr;
  if (shad.type() == shad_enum) {
    ptr = shad.get();
  }
  return typed_shader_view<shad_enum>{ptr};
}

template<shader_type shad_enum>
class typed_shader : public impl::rshader_owning<typed_shader<shad_enum>> {
public:
  template<shader_type _shad_enum>
  friend typed_shader<_shad_enum> to_typed(shader&& shad) noexcept;

public:
  typed_shader() noexcept :
    impl::rshader_owning<typed_shader<shad_enum>>{nullptr} {}

private:
  typed_shader(shader_t shad) noexcept :
    impl::rshader_owning<typed_shader<shad_enum>>{shad} {}

public:
  static expect<typed_shader> create(context_view ctx, cspan<cstring_view<char>> src) {
    return ntfr::create_shader(ctx, {
      .type = shad_enum,
      .source = src,
    })
    .transform([](shader_t shad) -> typed_shader {
      return typed_shader{shad};
    });
  }

public:
  operator shader_view() const noexcept { return {this->get()}; }
  operator typed_shader_view<shad_enum>() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
typed_shader<shad_enum> to_typed(shader&& shad) noexcept {
  shader_t ptr = nullptr;
  if (shad.type() == shad_enum) {
    ptr = shad.release();
  }
  return typed_shader<shad_enum>{ptr};
}

using vertex_shader = typed_shader<shader_type::vertex>;
using vertex_shader_view = typed_shader_view<shader_type::vertex>;

using fragment_shader = typed_shader<shader_type::fragment>;
using fragment_shader_view = typed_shader_view<shader_type::fragment>;

using geometry_shader = typed_shader<shader_type::geometry>;
using geometry_shader_view = typed_shader_view<shader_type::geometry>;

using tess_control_shader = typed_shader<shader_type::tesselation_control>;
using tess_control_shader_view = typed_shader_view<shader_type::tesselation_control>;

using tess_eval_shader = typed_shader<shader_type::tesselation_eval>;
using tess_eval_shader_view = typed_shader_view<shader_type::tesselation_eval>;

using compute_shader = typed_shader<shader_type::compute>;
using compute_shader_view = typed_shader_view<shader_type::compute>;

} // namespace ntf


namespace ntf::impl {

template<typename Derived>
class rtexture_ops {
  ntfr::texture_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::texture_t ptr =  static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid texture handle");
    return ptr;
  }

public:
  operator ntfr::texture_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::expect<void> upload(const ntfr::texture_data& data) const {
    return ntfr::texture_upload(_ptr(), data);
  }
  ntfr::expect<void> sampler(ntfr::texture_sampler sampler) const {
    return ntfr::texture_set_sampler(_ptr(), sampler);
  }
  ntfr::expect<void> addressing(ntfr::texture_addressing addressing) const {
    return ntfr::texture_set_addressing(_ptr(), addressing);
  }

  ntfr::context_view context() const {
    return {ntfr::texture_get_ctx(_ptr())};
  }
  ntfr::texture_type type() const {
    return ntfr::texture_get_type(_ptr());
  }
  ntfr::image_format format() const {
    return ntfr::texture_get_format(_ptr());
  }
  ntfr::texture_sampler sampler() const {
    return ntfr::texture_get_sampler(_ptr());
  }
  ntfr::texture_addressing addressing() const {
    return ntfr::texture_get_addressing(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::texture_get_id(_ptr());
  }
  ntfr::extent3d extent() const {
    return ntfr::texture_get_extent(_ptr());
  }
  uint32 layers() const {
    return ntfr::texture_get_layers(_ptr());
  }
  uint32 levels() const {
    return ntfr::texture_get_levels(_ptr());
  }
  bool is_cubemap() const {
    return type() == ntfr::texture_type::cubemap;
  }
  bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  bool has_mipmaps() const {
    return levels() > 0;
  }
};

template<typename Derived>
class rtexture_view : public rtexture_ops<Derived> {
protected:
  rtexture_view(ntfr::texture_t tex) noexcept :
    _tex{tex} {}

public:
  ntfr::texture_t get() const noexcept { return _tex; }

  bool empty() const noexcept { return _tex == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::texture_t _tex;
};

template<typename Derived>
class rtexture_owning : public rtexture_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::texture_t tex) noexcept {
      ntfr::destroy_texture(tex);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::texture_t>, deleter_t>;

protected:
  rtexture_owning(ntfr::texture_t tex) noexcept :
    _tex{tex} {}

public:
  ntfr::texture_t get() const noexcept { return _tex.get(); }
  [[nodiscard]] ntfr::texture_t release() noexcept { return _tex.release(); }

  bool empty() const noexcept { return _tex.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _tex;
};

} // namespace ntf::impl

namespace ntf::render {

class texture_view : public ntf::impl::rtexture_view<texture_view> {
public:
  texture_view() noexcept :
    ntf::impl::rtexture_view<texture_view>{nullptr} {}

  texture_view(texture_t tex) noexcept :
    ntf::impl::rtexture_view<texture_view>{tex} {}
};

class texture : public ntf::impl::rtexture_owning<texture> {
public:
  texture() noexcept :
    ntf::impl::rtexture_owning<texture>{nullptr} {}

  explicit texture(texture_t tex) noexcept :
    ntf::impl::rtexture_owning<texture>{tex} {}

public:
  static expect<texture> create(context_view ctx, const texture_desc& desc) {
    return ntfr::create_texture(ctx.get(), desc)
    .transform([](texture_t tex) -> texture {
      return texture{tex};
    });
  }

public:
  operator texture_view() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
class typed_texture;

template<texture_type tex_enum>
class typed_texture_view : public ntf::impl::rtexture_view<typed_texture_view<tex_enum>> {
private:
  friend typed_texture<tex_enum>;

public:
  template<texture_type _tex_enum>
  friend typed_texture_view<_tex_enum> to_typed(texture_view tex) noexcept;

public:
  typed_texture_view() noexcept :
    ntf::impl::rtexture_view<typed_texture_view<tex_enum>>{nullptr} {}

private:
  typed_texture_view(texture_t tex) noexcept :
    ntf::impl::rtexture_view<typed_texture_view<tex_enum>>{tex} {}

public:
  operator texture_view() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
typed_texture_view<tex_enum> to_typed(texture_view tex) noexcept {
  texture_t ptr = nullptr;
  if (tex.type() == tex_enum) {
    ptr = tex.get();
  }
  return typed_texture_view<tex_enum>{ptr};
}

template<texture_type tex_enum>
class typed_texture : public ntf::impl::rtexture_owning<typed_texture<tex_enum>> {
public:
  template<texture_type _tex_enum>
  friend typed_texture<_tex_enum> to_typed(texture&& tex) noexcept;

public:
  typed_texture() noexcept :
    ntf::impl::rtexture_owning<typed_texture<tex_enum>>{nullptr} {}

private:
  typed_texture(texture_t tex) noexcept :
    ntf::impl::rtexture_owning<typed_texture<tex_enum>>{tex} {}

public:
  static expect<typed_texture> create(context_view ctx, const typed_texture_desc& desc) {
    return ntfr::create_texture(ctx.get(), {
      .type = tex_enum,
      .format = desc.format,
      .sampler = desc.sampler,
      .addressing = desc.addressing,
      .extent = desc.extent,
      .layers = desc.layers,
      .levels = desc.levels,
      .data = desc.data,
    })
    .transform([](texture_t tex) -> typed_texture {
      return typed_texture{tex};
    });
  }

public:
  operator texture_view() const noexcept { return {this->get()}; }
  operator typed_texture_view<tex_enum>() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
typed_texture<tex_enum> to_typed(texture&& tex) noexcept {
  texture_t ptr = nullptr;
  if (tex.type() == tex_enum) {
    ptr = tex.release();
  }
  return typed_texture<tex_enum>{ptr};
}

using texture1d = typed_texture<texture_type::texture1d>;
using texture1d_view = typed_texture_view<texture_type::texture1d>;

using texture2d = typed_texture<texture_type::texture2d>;
using texture2d_view = typed_texture_view<texture_type::texture2d>;

using texture3d = typed_texture<texture_type::texture3d>;
using texture3d_view = typed_texture_view<texture_type::texture3d>;

using cubemap_texture = typed_texture<texture_type::cubemap>;
using cubemap_texture_view = typed_texture_view<texture_type::cubemap>;

} // namespace ntf::render


namespace ntf::render {

class imgui_ctx {
private:
  imgui_ctx(context_view win, context_api shogle_api) noexcept;

public:
  imgui_ctx(imgui_ctx&& other) noexcept;
  imgui_ctx(const imgui_ctx&) = delete;

  ~imgui_ctx() noexcept;

public:
  static imgui_ctx create(context_view ctx, window_t win,
                          ImGuiConfigFlags flags = ImGuiConfigFlags_NavEnableKeyboard,
                          bool bind_callbacks = true);

public:
  imgui_ctx& operator=(imgui_ctx&& other) noexcept;
  imgui_ctx& operator=(const imgui_ctx&) = delete;

public:
  void operator()(context_t, ctx_handle);

public:
  void start_frame();
  void end_frame(framebuffer_view target = nullptr, 
                 uint32 sort_group = 0u,
                 weak_cptr<external_state> state = nullptr,
                 ImDrawData* draw_data = nullptr);

private:
  void _destroy() noexcept;

private:
  context_view _ctx;
  context_api _shogle_api;
  ImDrawData* _draw_data;
};

} // namespace ntf::render

namespace ntf::render {

constexpr uint32 BONE_TOMBSTONE = std::numeric_limits<uint32>::max();
template<size_t num_weights>
struct vertex_weights {
  vertex_weights() noexcept {
    for (uint32 i = 0; i < num_weights; ++i) {
      indices[i] = BONE_TOMBSTONE;
    }
  }

  uint32 indices[num_weights];
  f32 weights[num_weights];
};

template<size_t num_weights>
struct soa_vertices {
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<vec2> uvs;
  std::vector<vec3> tangents;
  std::vector<vec3> bitangents;
  std::vector<vertex_weights<num_weights>> weights;
  std::vector<color4> colors;
};

} // namespace ntf::render

namespace ntf::meta {

template<typename Vertex>
concept vert_has_positions = requires(Vertex vert, const vec3& pos) {
  vert.set_position(pos);
};

template<typename Vertex>
concept vert_has_normals = requires(Vertex vert, const vec3& norm) {
  vert.set_normal(norm);
};

template<typename Vertex>
concept vert_has_uvs = requires(Vertex vert, const vec2& uv) {
  vert.set_uv(uv);
};

template<typename Vertex>
concept vert_has_tangents = requires(Vertex vert, const vec3& tang, const vec3& bitang) {
  vert.set_tangent(tang);
  vert.set_bitangent(bitang);
};

template<typename Vertex, size_t num_weights>
concept vert_has_weights = requires(Vertex vert, const ntfr::vertex_weights<num_weights>& weight) {
  vert.set_weights(weight);
};

template<typename Vertex>
concept vert_has_colors = requires(Vertex vert, const ntfr::color4& color) {
  vert.set_color(color);
};

template<typename>
struct check_soa_vertex : public std::false_type {};

template<size_t w>
struct check_soa_vertex<ntfr::soa_vertices<w>> : public std::true_type {};

template<typename T>
concept is_soa_vertex = check_soa_vertex<T>::value;

template<typename T>
concept is_aos_vertex = vert_has_positions<T>;

template<typename T>
concept is_vertex_type = is_soa_vertex<T> || is_aos_vertex<T>;

} // namespacen ntf::meta

namespace ntf::render {

struct pn_vertex {
  vec3 position;
  vec3 normal;

  [[nodiscard]] static constexpr std::array<attribute_binding, 2u> aos_binding() {
    // [ pos_0, norm_0 | pos_1, norm_1 | ... | pos_n-1, norm_n-1 ]
    std::array<attribute_binding, 2> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].type = attribute_type::vec3;
    desc[0].offset = offsetof(pn_vertex, position);
    desc[0].stride = sizeof(pn_vertex);
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    desc[1].type = attribute_type::vec3;
    desc[1].offset = offsetof(pn_vertex, normal);
    desc[1].stride = sizeof(pn_vertex);
    desc[1].location = 1;

    return desc;
  }

  [[nodiscard]] static constexpr std::array<attribute_binding, 2u> soa_binding() {
    // [ pos_0, pos_1, ..., pos_n-1 | norm_0, norm_1, ..., norm_n-1 ]
    // vertex_count == n
    std::array<attribute_binding, 2> desc;

    // layout (location = 0) in vec3 att_position;
    // constexpr size_t pos_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[0].type = attribute_type::vec3;
    desc[0].offset = 0u;
    desc[0].stride = 0u;
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    // constexpr size_t norm_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[1].type = attribute_type::vec3;
    desc[1].offset = 0u;
    desc[1].stride = 0u;
    desc[1].location = 1;

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
};
static_assert(
  meta::is_aos_vertex<pn_vertex> &&
  meta::vert_has_positions<pn_vertex> &&
  meta::vert_has_normals<pn_vertex>
);

struct pnt_vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;

  [[nodiscard]] static constexpr std::array<attribute_binding, 3u> aos_binding() {
    // [ pos_0, norm_0, uv_0 | pos_1, norm_1, uv_1 | ... | pos_n-1, norm_n-1, uv_n-1 ]
    std::array<attribute_binding, 3u> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].type = attribute_type::vec3;
    desc[0].offset = offsetof(pnt_vertex, position);
    desc[0].stride = sizeof(pnt_vertex);
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    desc[1].type = attribute_type::vec3;
    desc[1].offset = offsetof(pnt_vertex, normal);
    desc[1].stride = sizeof(pnt_vertex);
    desc[1].location = 1;

    // layout (location = 2) in vec2 att_uv;
    desc[2].type = attribute_type::vec2;
    desc[2].offset = offsetof(pnt_vertex, uv);
    desc[2].stride = sizeof(pnt_vertex);
    desc[2].location = 2;

    return desc;
  }

  [[nodiscard]] static constexpr std::array<attribute_binding, 3u> soa_binding() {
    // [ pos_0, pos_1, ..., pos_n-1 | norm_0, norm_1, ..., norm_n-1 | uv_0, uv_1, ..., uv_n-1 ]
    // vertex_count == n
    std::array<attribute_binding, 3u> desc;

    // layout (location = 0) in vec3 att_position;
    // constexpr size_t pos_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[0].type = attribute_type::vec3;
    desc[0].offset = 0;
    desc[0].stride = 0;
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    // constexpr size_t norm_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[1].type = attribute_type::vec3;
    desc[1].offset = 0;
    desc[1].stride = 0;
    desc[1].location = 1;

    // layout (location = 2) in vec2 att_uv;
    // constexpr size_t uv_stride = 2*meta::attribute_size(attribute_type::vec2);
    desc[2].type = attribute_type::vec2;
    desc[2].offset = 0;
    desc[2].stride = 0;
    desc[2].location = 2;

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
  void set_uv(const vec2& uvs) { uv = uvs; }
};
static_assert(
  meta::is_aos_vertex<pnt_vertex> &&
  meta::vert_has_positions<pnt_vertex> &&
  meta::vert_has_normals<pnt_vertex> &&
  meta::vert_has_uvs<pnt_vertex>
);

} // namespace ntf::render
