#pragma once

#include "./state.hpp"

namespace ntf {

class gl_context final : public rp_context {
private:
  template<typename T, typename H>
  class res_container {
  public:
    res_container(rp_alloc& alloc) noexcept :
      _res(alloc.make_adaptor<T>()), _free(alloc.make_adaptor<r_handle_value>()) {}

  public:
    template<typename Fun>
    void clear(Fun&& f) {
      for (auto& res : _res) {
        f(res);
      }
      _res.clear();
      _free = {};
    }

    H acquire() {
      if (!_free.empty()) {
        H pos{_free.front()};
        _free.pop();
        return pos;
      }
      _res.emplace_back(T{});
      return H{static_cast<r_handle_value>(_res.size()-1)};
    }

    void push(H pos) {
      NTF_ASSERT(validate(pos));
      _free.push(static_cast<r_handle_value>(pos));
    }

    T& get(H pos) {
      NTF_ASSERT(validate(pos));
      return _res[static_cast<r_handle_value>(pos)];
    }

    const T& get(H pos) const {
      NTF_ASSERT(validate(pos));
      return _res[static_cast<r_handle_value>(pos)];
    }

    bool validate(H pos) const {
      return static_cast<r_handle_value>(pos) < _res.size();
    }

  private:
    rp_alloc::vec_t<T> _res;
    rp_alloc::queue_t<r_handle_value> _free;
  };

public:
  gl_context(rp_alloc& alloc, r_window win, uint32 swap_interval) noexcept;

public:
  void get_meta(rp_platform_meta& meta) override;

  r_platform_buffer create_buffer(const rp_buff_desc& desc) override;
  void update_buffer(r_platform_buffer buf, const rp_buff_data& data) override;
  void* map_buffer(r_platform_buffer buf, const rp_buff_mapping& mapping) override;
  void unmap_buffer(r_platform_buffer buf, void* ptr) noexcept override;
  void destroy_buffer(r_platform_buffer buff) noexcept override;

  r_platform_texture create_texture(const rp_tex_desc&) override;
  void upload_texture_images(r_platform_texture tex,
                             cspan<rp_tex_image_data> images, bool regen_mips) override;
  void update_texture_options(r_platform_texture tex, const rp_tex_opts& otps) override;
  void destroy_texture(r_platform_texture tex) noexcept override;

  r_platform_shader create_shader(const rp_shad_desc& desc) override;
  void destroy_shader(r_platform_shader shader) noexcept override;

  r_platform_pipeline create_pipeline(const rp_pip_desc& desc) override;
  void update_pipeline_options(r_platform_pipeline pip, const rp_pip_opts& opts) override;
  void destroy_pipeline(r_platform_pipeline pipeline) noexcept override;

  r_platform_fbo create_framebuffer(const rp_fbo_desc& desc) override;
  void destroy_framebuffer(r_platform_fbo fb) noexcept override;

  void submit(r_context ctx, cspan<rp_draw_data> draw_data) override;
  void swap_buffers() override;

private:
  GLAPIENTRY static void debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                        GLsizei len, const GLchar* msg, const void* user_ptr);

private:
  rp_alloc& _alloc;
  r_window _win;
  uint32 _swap_interval;

  gl_state _state;
  gl_state::vao_t _vao;

  res_container<gl_state::buffer_t, r_platform_buffer> _buffers;
  res_container<gl_state::texture_t, r_platform_texture> _textures;
  res_container<gl_state::shader_t, r_platform_shader> _shaders;
  res_container<gl_state::program_t, r_platform_pipeline> _programs;
  res_container<gl_state::framebuffer_t, r_platform_fbo> _framebuffers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);
};

} // namespace ntf
