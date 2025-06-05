#pragma once

#include "../platform.hpp"

#ifdef SHOGLE_GL_DISABLE_ASSERTIONS
#define GL_CALL(fun, ...) fun(__VA_ARGS__)
#define GL_CALL_RET(fun, ...) fun(__VA_ARGS__)
#else
#define GL_CALL(fun, ...) \
do { \
  fun(__VA_ARGS__); \
  GLenum glerr = ::ntf::gl_state::check_error(NTF_FILE, NTF_FUNC, NTF_LINE); \
  NTF_ASSERT(glerr == 0, "GL ERROR: {}", glerr); \
} while(0) 
#define GL_CALL_RET(fun, ...) \
[&]() { \
  auto ret = fun(__VA_ARGS__); \
  GLenum glerr = ::ntf::gl_state::check_error(NTF_FILE, NTF_FUNC, NTF_LINE); \
  NTF_ASSERT(glerr == 0, "GL ERROR: {}", glerr); \
  return ret; \
}()
#endif

#define GL_CHECK(fun, ...) \
[&]() { \
  fun(__VA_ARGS__); \
  return ::ntf::gl_state::check_error(NTF_FILE, NTF_FUNC, NTF_LINE); \
}()

namespace ntf::render {

class gl_context;

class gl_state {
public:
  static constexpr uint32 MAX_FBO_ATTACHMENTS = 8;
  static constexpr uint32 MAX_TEXTURE_LEVEL = 7;
  static constexpr GLuint NULL_BINDING = 0;
  static constexpr GLuint DEFAULT_FBO = 0;

public:
  struct buffer_t {
    GLuint id{0};
    GLenum type;
    GLbitfield flags;
    size_t size;
    void* mapping_ptr;
    GLbitfield mapping;
  };

  enum buffer_bindings {
    BUFFER_TYPE_VERTEX = 0,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_TEXEL,
    BUFFER_TYPE_UNIFORM,
    BUFFER_TYPE_SHADER,

    BUFFER_TYPE_COUNT,
  };

  struct vao_t {
    GLuint id{0};
  };

  struct shader_t {
    GLuint id{0};
    GLenum type;
  };

  enum program_flags : uint8 {
    PROG_ENABLE_NOTHING = 0,
    PROG_ENABLE_CULLING = 1<<0,
    PROG_ENABLE_BLENDING = 1<<1,
    PROG_ENABLE_STENCIL = 1<<2,
    PROG_ENABLE_DEPTH = 1<<3,
    PROG_ENABLE_SCISSOR = 1<<4,
  };

  struct program_t {
    GLuint id{0};
    GLenum primitive;
    GLenum poly;
    float poly_width;

    uint8 flags;
    struct {
      GLenum mode;
      GLenum face;
    } culling;

    struct {
      GLenum mode;
      GLenum src_fac;
      GLenum dst_fac;
      float r, g, b, a;
    } blending;

    struct {
      GLenum func;
      int32 func_ref;
      uint32 func_mask;

      GLenum sfail;
      GLenum dpfail;
      GLenum dppass;

      uint32 mask;
    } stencil;

    struct {
      GLenum func;
      double near, far;
    } depth;

    struct {
      uint32 x, y;
      uint32 w, h;
    } scissor;

    cspan<attribute_binding> layout;
  };

  struct texture_t {
    GLuint id{0};
    GLenum type;
    GLenum format;
    GLenum sampler[2]; // MIN, MAG
    GLenum addressing;
    uvec3 extent;
    uint32 layers;
    uint32 levels;
  };

  struct framebuffer_t {
    GLuint id{0};
    uvec2 extent;
    GLuint sd_rbo;
    GLuint color_rbo;
  };

  enum fbo_binding {
    FBO_BIND_READ = 0,
    FBO_BIND_WRITE,

    FBO_BIND_COUNT,
    FBO_BIND_BOTH,
  };

  struct init_data_t {
    GLDEBUGPROC dbg;
    gl_context* ctx;
  };

  struct fbo_attachment_t {
    texture_t* tex;
    uint32 layer;
    uint32 level;
  };

public:
  gl_state(ctx_alloc& alloc) noexcept;

public:
  void init(const init_data_t& data) noexcept;

public:
  void create_buffer(buffer_t& buffer, buffer_type type, buffer_flag flags,
                         size_t size, weak_cptr<buffer_data> data);
  void destroy_buffer(buffer_t& buffer);
  bool buffer_bind(GLuint id, GLenum type);
  void buffer_upload(buffer_t& buffer, size_t size, size_t offset, const void* data);
  void buffer_map(buffer_t& buffer, void** ptr, size_t size, size_t offset);
  void buffer_unmap(buffer_t& buffer, void* ptr);

  void create_vao(vao_t& vao);
  void destroy_vao(vao_t& vao);
  bool bind_vao(GLuint id);

  void create_shader(shader_t& shad, shader_type type, std::string_view src);
  void destroy_shader(shader_t& shad);

  void create_program(program_t& prog,
                      cspan<shader_t*> shaders, primitive_mode primitive,
                      polygon_mode poly_mode, f32 poly_width,
                      render_tests tests);
  void destroy_program(program_t& prog);
  bool program_bind(GLuint id);
  void program_query_uniforms(program_t& prog, unif_meta_vec& unifs);
  void program_prepare_state(program_t& prog);
  void push_uniform(GLuint location, attribute_type tye, const void* data);

  void create_texture(texture_t& tex, texture_type type, image_format format,
                      texture_sampler sampler, texture_addressing addressing,
                      extent3d extent, uint32 layers, uint32 levels);
  void destroy_texture(texture_t& tex);
  bool texture_bind(GLuint tex, GLenum type, uint32 index);
  void texture_upload(texture_t& tex, const uint8* texels,
                      image_format image, image_alignment alignment,
                      extent3d offset, uint32 layer, uint32 level);
  void texture_set_sampler(texture_t& tex, texture_sampler sampler);
  void texture_set_addressing(texture_t& tex, texture_addressing addressing);
  void texture_gen_mipmaps(texture_t& tex);

  // void create_framebuffer(framebuffer_t& fbo, extent2d extent,
  //                         fbo_buffer test_buffers, image_format format);
  void create_framebuffer(framebuffer_t& fbo, extent2d extent,
                          fbo_buffer test_buffers, cspan<ctx_fbo_desc::tex_att_t> attachments);
  void destroy_framebuffer(framebuffer_t& fbo);
  bool framebuffer_bind(GLuint id, fbo_binding binding); 
  void framebuffer_prepare_state(GLuint fbo, clear_flag flags,
                                 const uvec4& vp, const color4& color);
  
  void attribute_bind(cspan<attribute_binding> attrs);
  void prepare_state(const external_state& stat);

public:
  const auto& tex_limits() const { return _tex_limits; }

public:
  [[nodiscard]] static GLenum buffer_type_cast(buffer_type type) noexcept;

  [[nodiscard]] static GLenum shader_type_cast(shader_type type) noexcept;

  [[nodiscard]] static GLenum fbo_attachment_cast(fbo_buffer att) noexcept;

  [[nodiscard]] static GLenum texture_type_cast(texture_type type, bool is_array) noexcept;
  [[nodiscard]] static GLenum texture_format_cast(image_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_symbolic_cast(image_format format) noexcept;
  [[nodiscard]] static GLenum texture_format_underlying_cast(image_format format) noexcept;
  [[nodiscard]] static GLenum texture_sampler_cast(texture_sampler samp, bool mips) noexcept;
  [[nodiscard]] static GLenum texture_addressing_cast(texture_addressing address) noexcept;

  [[nodiscard]] static GLenum attrib_underlying_type_cast(attribute_type type) noexcept;
  [[nodiscard]] static GLbitfield clear_bit_cast(clear_flag flags) noexcept;

  [[nodiscard]] static attribute_type uniform_type_cast(GLenum type) noexcept;
  [[nodiscard]] static GLenum primitive_cast(primitive_mode type) noexcept;
  [[nodiscard]] static GLenum poly_mode_cast(polygon_mode poly_mode) noexcept;
  [[nodiscard]] static GLenum test_func_cast(test_func func) noexcept;
  [[nodiscard]] static GLenum stencil_op_cast(stencil_op op) noexcept;
  [[nodiscard]] static GLenum blend_func_cast(blend_factor func) noexcept;
  [[nodiscard]] static GLenum cull_mode_cast(cull_mode mode) noexcept;
  [[nodiscard]] static GLenum cull_face_cast(cull_face face) noexcept;
  [[nodiscard]] static GLenum blend_eq_cast(blend_mode eq) noexcept;

public:
  static GLenum check_error(const char* file, const char* func, int line) noexcept;

private:
  GLenum& _buffer_pos(GLenum type);

private:
  GLuint _bound_buffers[BUFFER_TYPE_COUNT];
  GLuint _bound_fbos[FBO_BIND_COUNT];
  GLuint _bound_vao;
  GLuint _bound_program;

  struct {
    uint32 max_dim;
    uint32 max_dim3d;
    uint32 max_layers;
  } _tex_limits;
  ctx_alloc::vec_t<std::pair<GLuint, GLenum>> _bound_texs; // id, type
  size_t _active_tex;
};

class gl_context final : public icontext {
private:
  template<typename T>
  class res_container {
  public:
    res_container(ctx_alloc& alloc) noexcept :
      _res(alloc.make_adaptor<T>()), _free(alloc.make_adaptor<ctx_handle>()) {}

  public:
    template<typename Fun>
    void clear(Fun&& f) {
      for (auto& res : _res) {
        f(res);
      }
      _res.clear();
      _free = {};
    }

    ctx_handle acquire() {
      if (!_free.empty()) {
        ctx_handle pos{_free.front()};
        _free.pop();
        return pos;
      }
      _res.emplace_back();
      return _res.size()-1u;
    }

    void push(ctx_handle pos) {
      NTF_ASSERT(validate(pos));
      _free.push(pos);
    }

    T& get(ctx_handle pos) {
      NTF_ASSERT(validate(pos));
      return _res[pos];
    }

    const T& get(ctx_handle pos) const {
      NTF_ASSERT(validate(pos));
      return _res[pos];
    }

    bool validate(ctx_handle pos) const {
      return pos < _res.size();
    }

  private:
    ctx_alloc::vec_t<T> _res;
    ctx_alloc::queue_t<ctx_handle> _free;
  };

public:
  gl_context(ctx_alloc& alloc, window_t win, uint32 swap_interval) noexcept;

public:
  void get_meta(ctx_meta& meta) override;

  ctx_buff_status create_buffer(ctx_buff& buff, const ctx_buff_desc& desc) override;
  ctx_buff_status update_buffer(ctx_buff buff, const buffer_data& data) override;
  ctx_buff_status map_buffer(ctx_buff buff, void** ptr, size_t size, size_t offset) override;
  ctx_buff_status unmap_buffer(ctx_buff buff, void* ptr) noexcept override;
  ctx_buff_status destroy_buffer(ctx_buff buff) noexcept override;

  ctx_tex_status create_texture(ctx_tex& tex, const ctx_tex_desc& desc) override;
  ctx_tex_status update_texture(ctx_tex tex, const ctx_tex_data& data) override;
  ctx_tex_status update_texture(ctx_tex tex, const ctx_tex_opts& opts) override;
  ctx_tex_status destroy_texture(ctx_tex tex) noexcept override;

  ctx_shad_status create_shader(ctx_shad& shad, const ctx_shad_desc& desc) override;
  ctx_shad_status destroy_shader(ctx_shad shad) override;

  ctx_pip_status create_pipeline(ctx_pip& pip, const ctx_pip_desc& desc) override;
  ctx_pip_status destroy_pipeline(ctx_pip pip) override;

  ctx_fbo_status create_framebuffer(ctx_fbo& fbo, const ctx_fbo_desc& desc) override;
  ctx_fbo_desc destroy_framebuffer(ctx_fbo fbo) noexcept override;

  void submit_render_data(context_t ctx, cspan<ctx_render_data> render_data) override;

  void device_wait() override;
  void swap_buffers() override;

private:
  GLAPIENTRY static void debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                        GLsizei len, const GLchar* msg, const void* user_ptr);

private:
  ctx_alloc& _alloc;
  window_t _win;
  uint32 _swap_interval;

  gl_state _state;
  gl_state::vao_t _vao;

  res_container<gl_state::buffer_t> _buffers;
  res_container<gl_state::texture_t> _textures;
  res_container<gl_state::shader_t> _shaders;
  res_container<gl_state::program_t> _programs;
  res_container<gl_state::framebuffer_t> _framebuffers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);
};

} // namespace ntf::render
