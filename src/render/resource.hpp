#pragma once

#include "./context.hpp"

namespace ntf {

class r_framebuffer_view;

class r_context_view {
public:
  r_context_view(r_context ctx) noexcept :
    _ctx{ctx} {}

public:
  r_context handle() const { return _ctx; }
  r_framebuffer_view default_fbo() const;

  void start_frame() const {
    r_start_frame(_ctx);
  }
  void end_frame() const {
    r_end_frame(_ctx);
  }
  void device_wait() const {
    r_device_wait(_ctx);
  }
  void submit_command(const r_draw_command& cmd) const {
    r_submit_command(_ctx, cmd);
  }

protected:
  r_context _ctx;
};

class renderer_context : public r_context_view {
private:
  struct deleter_t {
    void operator()(r_context ctx) {
      r_destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<r_context_, deleter_t>;

public:
  explicit renderer_context(r_context ctx) noexcept :
    r_context_view{ctx},
    _handle{ctx} {}

public:
  static auto create(
    const r_context_params& params
  ) noexcept -> r_expected<renderer_context>
  {
    return r_create_context(params)
      .transform([](r_context ctx) -> renderer_context {
        return renderer_context{ctx};
      });
  }

private:
  uptr_type _handle;
};


class r_buffer_view {
public:
  r_buffer_view(r_buffer buffer) noexcept :
    _buffer{buffer} {}

public:
  r_expected<void> upload(size_t offset, size_t len, const void* data) const {
    return r_buffer_upload(_buffer, offset, len, data);
  }
  void upload(unchecked_t,
              size_t offset, size_t len, const void* data) const {
    r_buffer_upload(::ntf::unchecked, _buffer, offset, len, data);
  }
  r_expected<void*> map(size_t offset, size_t len) const {
    return r_buffer_map(_buffer, offset, len);
  }
  void* map(unchecked_t, size_t offset, size_t len) const {
    return r_buffer_map(::ntf::unchecked, _buffer, offset, len);
  }
  void unmap(void* ptr) const {
    r_buffer_unmap(_buffer, ptr);
  }

public:
  r_buffer handle() const { return _buffer; }
  r_context_view context() const { return r_buffer_get_ctx(_buffer); }

  r_buffer_type type() const {
    return r_buffer_get_type(_buffer);
  }
  size_t size() const {
    return r_buffer_get_size(_buffer);
  }

protected:
  r_buffer _buffer;
};

class renderer_buffer : public r_buffer_view {
private:
  struct deleter_t {
    void operator()(r_buffer buff) {
      r_destroy_buffer(buff);
    }
  };
  using uptr_type = std::unique_ptr<r_buffer_, deleter_t>;

public:
  explicit renderer_buffer(r_buffer buffer) noexcept :
    r_buffer_view{buffer},
    _handle{buffer} {}

public:
  static auto create(
    r_context_view ctx, const r_buffer_descriptor& desc
  ) noexcept -> r_expected<renderer_buffer>
  {
    return r_create_buffer(ctx.handle(), desc)
      .transform([](r_buffer buff) -> renderer_buffer {
        return renderer_buffer{buff};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_buffer_descriptor& desc
  ) -> renderer_buffer
  {
    return renderer_buffer{r_create_buffer(::ntf::unchecked, ctx.handle(), desc)};
  }

  template<typename U, size_t N>
  static auto create(
    r_context_view ctx, U (&arr)[N], r_buffer_type type, r_buffer_flag flags
  ) noexcept -> r_expected<renderer_buffer>
  {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0u,
    };
    return renderer_buffer::create(ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata,
    });
  }

  template<typename U, size_t N>
  static auto create(
    unchecked_t,
    r_context_view ctx, U (&arr)[N], r_buffer_type type, r_buffer_flag flags
  ) -> renderer_buffer
  {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0u,
    };
    return renderer_buffer::create(::ntf::unchecked, ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata,
    });
  }

public:
  uptr_type _handle;
};


class r_texture_view {
public:
  r_texture_view(r_texture tex) noexcept :
    _tex{tex} {}

public:
  r_expected<void> upload(const r_texture_data& data) const {
    return r_texture_upload(_tex, data);
  }
  void upload(unchecked_t, const r_texture_data& data) const {
    r_texture_upload(::ntf::unchecked, _tex, data);
  }
  r_expected<void> upload(span_view<r_image_data> images, bool gen_mips) const {
    return r_texture_upload(_tex, images, gen_mips);
  }
  void upload(unchecked_t, span_view<r_image_data> images, bool gen_mips) const {
    r_texture_upload(::ntf::unchecked, _tex, images, gen_mips);
  }
  r_expected<void> sampler(r_texture_sampler sampler) const {
    return r_texture_set_sampler(_tex, sampler);
  }
  void sampler(unchecked_t, r_texture_sampler sampler) const {
    r_texture_set_sampler(::ntf::unchecked, _tex, sampler);
  }
  r_expected<void> addressing(r_texture_address addressing) const {
    return r_texture_set_addressing(_tex, addressing);
  }
  void addressing(unchecked_t, r_texture_address addressing) const {
    r_texture_set_addressing(::ntf::unchecked, _tex, addressing);
  }

public:
  r_texture handle() const { return _tex; }
  r_context_view context() const { return r_texture_get_ctx(_tex); }

  r_texture_type type() const {
    return r_texture_get_type(_tex);
  }
  r_texture_format format() const {
    return r_texture_get_format(_tex);
  }
  r_texture_sampler sampler() const {
    return r_texture_get_sampler(_tex);
  }
  r_texture_address addressing() const {
    return r_texture_get_addressing(_tex);
  }
  extent3d extent() const {
    return r_texture_get_extent(_tex);
  }
  uint32 layers() const {
    return r_texture_get_layers(_tex);
  }
  uint32 levels() const {
    return r_texture_get_levels(_tex);
  }
  bool is_cubemap() const {
    return type() == r_texture_type::cubemap;
  }
  bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  bool has_mipmaps() const {
    return levels() > 0;
  }

protected:
  r_texture _tex;
};

class renderer_texture : public r_texture_view {
private:
  struct deleter_t {
    void operator()(r_texture tex) { r_destroy_texture(tex); }
  };
  using uptr_type = std::unique_ptr<r_texture_, deleter_t>;

public:
  explicit renderer_texture(r_texture tex) noexcept :
    r_texture_view{tex},
    _handle{tex} {}

public:
  static auto create(
    r_context_view ctx, const r_texture_descriptor& desc
  ) noexcept -> r_expected<renderer_texture>
  {
    return r_create_texture(ctx.handle(), desc)
      .transform([](r_texture tex) -> renderer_texture {
        return renderer_texture{tex};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_texture_descriptor& desc
  ) -> renderer_texture
  {
    return renderer_texture{r_create_texture(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};


class r_framebuffer_view {
public:
  r_framebuffer_view(r_framebuffer fbo) noexcept :
    _fbo{fbo} {}

public:
  void clear_flags(r_clear_flag flags) const {
    r_framebuffer_set_clear(_fbo, flags);
  }
  void clear_color(const color4& color) const {
    r_framebuffer_set_color(_fbo, color);
  }
  void viewport(const uvec4& vp) const {
    r_framebuffer_set_viewport(_fbo, vp);
  }
  
public:
  r_framebuffer handle() const { return _fbo; }
  r_context_view context() const {
    return r_framebuffer_get_ctx(_fbo);
  }

  r_clear_flag clear_flags() const {
    return r_framebuffer_get_clear(_fbo);
  }
  color4 clear_color() const {
    return r_framebuffer_get_color(_fbo);
  }
  uvec4 viewport() const {
    return r_framebuffer_get_viewport(_fbo);
  }

protected:
  r_framebuffer _fbo;
};

inline r_framebuffer_view r_context_view::default_fbo() const {
  return r_framebuffer_view{r_get_default_framebuffer(_ctx)};
}

class renderer_framebuffer : public r_framebuffer_view {
private:
  struct deleter_t {
    void operator()(r_framebuffer fbo) {
      r_destroy_framebuffer(fbo);
    }
  };
  using uptr_type = std::unique_ptr<r_framebuffer_, deleter_t>;

public:
  explicit renderer_framebuffer(r_framebuffer fbo) noexcept :
    r_framebuffer_view{fbo},
    _handle{fbo} {}

public:
  static auto create(
    r_context_view ctx, const r_framebuffer_descriptor& desc
  ) noexcept -> r_expected<renderer_framebuffer>
  {
    return r_create_framebuffer(ctx.handle(), desc)
      .transform([](r_framebuffer fbo)-> renderer_framebuffer {
        return renderer_framebuffer{fbo};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_framebuffer_descriptor& desc
  ) -> renderer_framebuffer
  {
    return renderer_framebuffer{r_create_framebuffer(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};


class r_shader_view {
public:
  r_shader_view(r_shader shader) noexcept :
    _shader{shader} {}

public:
  r_shader handle() const { return _shader; }
  r_context_view context() const {
    return r_shader_get_ctx(_shader);
  }

  r_shader_type type() const {
    return r_shader_get_type(_shader);
  }

protected:
  r_shader _shader;
};

class renderer_shader : public r_shader_view {
private:
  struct deleter_t {
    void operator()(r_shader shader) {
      r_destroy_shader(shader);
    }
  };
  using uptr_type = std::unique_ptr<r_shader_, deleter_t>;

public:
  explicit renderer_shader(r_shader shader) noexcept :
    r_shader_view{shader},
    _handle{shader} {}

public:
  static auto create(
    r_context_view ctx, const r_shader_descriptor& desc
  ) -> r_expected<renderer_shader>
  {
    return r_create_shader(ctx.handle(), desc)
      .transform([](r_shader shad) -> renderer_shader {
        return renderer_shader{shad};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_shader_descriptor& desc
  ) -> renderer_shader
  {
    return renderer_shader{r_create_shader(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};


class r_pipeline_view {
public:
  r_pipeline_view(r_pipeline pip) noexcept :
    _pip{pip} {}

public:
  r_pipeline handle() const { return _pip; }
  r_context_view context() const { return r_pipeline_get_ctx(_pip); }

  r_stages_flag stages() const {
    return r_pipeline_get_stages(_pip);
  }
  optional<r_uniform> uniform(std::string_view name) const {
    return r_pipeline_get_uniform(_pip, name);
  }
  r_uniform uniform(unchecked_t, std::string_view name) const {
    return r_pipeline_get_uniform(::ntf::unchecked, _pip, name);
  }

protected:
  r_pipeline _pip;
};

class renderer_pipeline : public r_pipeline_view {
private:
  struct deleter_t {
    void operator()(r_pipeline pip) {
      r_destroy_pipeline(pip);
    }
  };
  using uptr_type = std::unique_ptr<r_pipeline_, deleter_t>;

public:
  explicit renderer_pipeline(r_pipeline pip) noexcept :
    r_pipeline_view{pip},
    _handle{pip} {}

public:
  static auto create(
    r_context_view ctx, const r_pipeline_descriptor& desc
  ) noexcept -> r_expected<renderer_pipeline>
  {
    return r_create_pipeline(ctx.handle(), desc)
      .transform([](r_pipeline pip) -> renderer_pipeline {
        return renderer_pipeline{pip};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_pipeline_descriptor& desc
  ) -> renderer_pipeline
  {
    return renderer_pipeline{r_create_pipeline(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};

} // namespace ntf
