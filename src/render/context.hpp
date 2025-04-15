#pragma once

#include "../platform_macros.hpp"

#include "./texture.hpp"
#include "./buffer.hpp"
#include "./pipeline.hpp"
#include "./framebuffer.hpp"

#include "../stl/optional.hpp"
#include "../stl/expected.hpp"

namespace ntf {

NTF_DECLARE_OPAQUE_HANDLE(r_context);

using r_error = ::ntf::error<void>;

template<typename T>
using r_expected = expected<T, r_error>;

struct r_allocator {
  void* user_ptr;
  void* (*mem_alloc)(void* user_ptr, size_t size, size_t alignment);
  void  (*mem_free)(void* user_ptr, void* mem);
  void* (*mem_scratch_alloc)(void* user_ptr, size_t size, size_t alignment);
  void  (*mem_scratch_free)(void* user_ptr, void* mem);
};

struct r_draw_opts {
  uint32 count;
  uint32 offset;
  uint32 instances;
  uint32 sort_group;
};

struct r_draw_command {
  r_framebuffer target;
  r_pipeline pipeline;
  span_view<r_buffer_binding> buffers;
  span_view<r_texture_binding> textures;
  span_view<r_push_constant> uniforms;
  weak_ref<r_draw_opts> draw_opts;
  std::function<void(r_context)> on_render;
};

struct r_context_params {
  win_handle window;
  renderer_api api;
  uint32 swap_interval;
  uvec4 fb_viewport;
  r_clear_flag fb_clear;
  color4 fb_color;
  const r_allocator* alloc; // Placeholder!!!
};

r_expected<r_context> r_create_context(const r_context_params& params);
void r_destroy_context(r_context ctx);

void r_start_frame(r_context ctx);
void r_end_frame(r_context ctx);
void r_device_wait(r_context ctx);
void r_submit_command(r_context ctx, const r_draw_command& cmd);

r_framebuffer r_get_default_framebuffer(r_context ctx);


r_expected<r_buffer> r_create_buffer(r_context ctx, const r_buffer_descriptor& desc);
r_buffer r_create_buffer(unchecked_t, r_context ctx, const r_buffer_descriptor& desc);
void r_destroy_buffer(r_buffer buffer);

r_expected<void> r_buffer_upload(r_buffer buff, size_t offset, size_t len, const void* data);
void r_buffer_upload(unchecked_t, r_buffer buff, size_t offset, size_t len, const void* data);
r_expected<void*> r_buffer_map(r_buffer buff, size_t offset, size_t len);
void* r_buffer_map(unchecked_t, r_buffer buff, size_t offset, size_t len);
void r_buffer_unmap(r_buffer buff, void* mapped);

r_buffer_type r_buffer_get_type(r_buffer buff);
size_t r_buffer_get_size(r_buffer buff);
r_context r_buffer_get_ctx(r_buffer buff);


r_expected<r_texture> r_create_texture(r_context ctx, const r_texture_descriptor& desc);
r_texture r_create_texture(unchecked_t, r_context ctx, const r_texture_descriptor& desc);
void r_destroy_texture(r_texture tex);

r_expected<void> r_texture_upload(r_texture tex, const r_texture_data& data);
void r_texture_upload(unchecked_t, r_texture tex, const r_texture_data& data);
r_expected<void> r_texture_upload(r_texture tex, span_view<r_image_data> images, bool gen_mips);
void r_texture_upload(unchecked_t, r_texture tex, span_view<r_image_data> images, bool gen_mips);
r_expected<void> r_texture_set_sampler(r_texture tex, r_texture_sampler sampler);
void r_texture_set_sampler(unchecked_t, r_texture tex, r_texture_sampler sampler);
r_expected<void> r_texture_set_addressing(r_texture tex, r_texture_address adressing);
void r_texture_set_addressing(unchecked_t, r_texture tex, r_texture_address addressing);

r_texture_type r_texture_get_type(r_texture tex);
r_texture_format r_texture_get_format(r_texture tex);
r_texture_sampler r_texture_get_sampler(r_texture tex);
r_texture_address r_texture_get_addressing(r_texture tex);
extent3d r_texture_get_extent(r_texture tex);
uint32 r_texture_get_layers(r_texture tex);
uint32 r_texture_get_levels(r_texture tex); 
r_context r_texture_get_ctx(r_texture tex);


r_expected<r_framebuffer> r_create_framebuffer(r_context ctx,
                                               const r_framebuffer_descriptor& desc);
                                               
r_framebuffer r_create_framebuffer(unchecked_t, r_context ctx,
                                   const r_framebuffer_descriptor& desc);
                                   
void r_destroy_framebuffer(r_framebuffer fbo);

void r_framebuffer_set_clear(r_framebuffer fbo, r_clear_flag flags);
void r_framebuffer_set_viewport(r_framebuffer fbo, const uvec4& vp);
void r_framebuffer_set_color(r_framebuffer fbo, const color4& color);

r_clear_flag r_framebuffer_get_clear(r_framebuffer fbo);
color4 r_framebuffer_get_color(r_framebuffer fbo);
uvec4 r_framebuffer_get_viewport(r_framebuffer fbo);

r_context r_framebuffer_get_ctx(r_framebuffer fbo);


r_expected<r_shader> r_create_shader(r_context ctx, const r_shader_descriptor& desc);
r_shader r_create_shader(unchecked_t, r_context ctx, const r_shader_descriptor& desc);
void r_destroy_shader(r_shader shader);

r_shader_type r_shader_get_type(r_shader shader);
r_context r_shader_get_ctx(r_shader shader);


r_expected<r_pipeline> r_create_pipeline(r_context ctx, const r_pipeline_descriptor& desc);
r_pipeline r_create_pipeline(unchecked_t, r_context ctx, const r_pipeline_descriptor& desc);
void r_destroy_pipeline(r_pipeline pip);

r_stages_flag r_pipeline_get_stages(r_pipeline pip);
optional<r_uniform> r_pipeline_get_uniform(r_pipeline pip, std::string_view name);
r_uniform r_pipeline_get_uniform(unchecked_t, r_pipeline pip, std::string_view name);

r_context r_pipeline_get_ctx(r_pipeline pip);


// template<typename T>
// requires(r_attrib_traits<T>::is_attrib)
// void r_push_uniform(r_uniform location, const T& data) {
//   NTF_ASSERT(location);
//   auto* desc = _data->frame_arena.allocate<r_context_data::uniform_descriptor>(1);
//   auto* data_ptr = _data->frame_arena.template allocate<T>(1);
//   std::construct_at(data_ptr, data);
//
//   desc->location = location;
//   desc->type = r_attrib_traits<T>::tag;
//   desc->data = data_ptr;
//   _data->d_cmd.uniforms.emplace_back(desc);
// }


// class r_context_view {
// public:
//   static constexpr r_framebuffer_handle DEFAULT_FRAMEBUFFER{};
//
// public:
//   r_context_view(weak_ref<r_context_data> data) noexcept :
//     _data{data} {}
//
// public:
//   void start_frame() noexcept;
//   void end_frame() noexcept;
//   void device_wait() noexcept;
//
//   void bind_texture(r_texture_handle texture, uint32 index) noexcept;
//   void bind_framebuffer(r_framebuffer_handle fbo) noexcept;
//   void bind_vertex_buffer(r_buffer_handle buffer) noexcept;
//   void bind_index_buffer(r_buffer_handle buffer) noexcept;
//   void bind_pipeline(r_pipeline_handle pipeline) noexcept;
//   void draw_opts(r_draw_opts opts) noexcept;
//   void submit() noexcept;
//   void submit(const r_draw_command& cmd) noexcept;
//
//   template<typename T>
//   requires(r_attrib_traits<T>::is_attrib)
//   void push_uniform(r_uniform location, const T& data) {
//     NTF_ASSERT(location);
//     auto* desc = _data->frame_arena.allocate<r_context_data::uniform_descriptor>(1);
//     auto* data_ptr = _data->frame_arena.template allocate<T>(1);
//     std::construct_at(data_ptr, data);
//
//     desc->location = location;
//     desc->type = r_attrib_traits<T>::tag;
//     desc->data = data_ptr;
//     _data->d_cmd.uniforms.emplace_back(desc);
//   }
//
// public:
//   [[nodiscard]] r_expected<r_buffer_handle> buffer_create(
//     const r_buffer_descriptor& desc
//   ) noexcept;
//
//   [[nodiscard]] r_buffer_handle buffer_create(
//     unchecked_t,
//     const r_buffer_descriptor& desc
//   );
//
//   void destroy(r_buffer_handle buff) noexcept;
//
//   r_expected<void> buffer_update(
//     r_buffer_handle buff,
//     const r_buffer_data& desc
//   ) noexcept;
//
//   void buffer_update(
//     unchecked_t,
//     r_buffer_handle buff,
//     const r_buffer_data& desc
//   );
//
//   r_expected<void*> buffer_map(r_buffer_handle buff, size_t offet, size_t len);
//   void* buffer_map(unchecked_t, r_buffer_handle buff, size_t offset, size_t len);
//
//   void buffer_unmap(r_buffer_handle buff, void* ptr);
//
//   [[nodiscard]] r_buffer_type buffer_type(r_buffer_handle buff) const;
//   [[nodiscard]] size_t buffer_size(r_buffer_handle buff) const;
//
// public:
//   [[nodiscard]] r_expected<r_texture_handle> texture_create(
//     const r_texture_descriptor& desc
//   ) noexcept;
//
//   [[nodiscard]] r_texture_handle texture_create(
//     unchecked_t,
//     const r_texture_descriptor& desc
//   );
//
//   void destroy(r_texture_handle tex) noexcept;
//
//   r_expected<void> texture_update(
//     r_texture_handle tex,
//     const r_texture_data& data
//   ) noexcept;
//
//   void texture_update(
//     unchecked_t,
//     r_texture_handle tex,
//     const r_texture_data& data
//   );
//
//   r_expected<void> texture_update(
//     r_texture_handle tex,
//     span_view<r_image_data> images,
//     bool gen_mipmaps
//   ) noexcept;
//
//   void texture_update(
//     unchecked_t,
//     r_texture_handle tex,
//     span_view<r_image_data> images,
//     bool gen_mipmaps
//   );
//
//   r_expected<void> texture_sampler(
//     r_texture_handle tex,
//     r_texture_sampler sampler
//   ) noexcept;
//
//   void texture_sampler(
//     unchecked_t,
//     r_texture_handle tex,
//     r_texture_sampler sampler
//   );
//
//   r_expected<void> texture_addressing(
//     r_texture_handle tex,
//     r_texture_address addressing
//   ) noexcept;
//
//   void texture_addressing(
//     unchecked_t,
//     r_texture_handle tex,
//     r_texture_address addressing
//   );
//
//   [[nodiscard]] r_texture_type texture_type(r_texture_handle tex) const;
//   [[nodiscard]] r_texture_format texture_format(r_texture_handle tex) const;
//   [[nodiscard]] r_texture_sampler texture_sampler(r_texture_handle tex) const;
//   [[nodiscard]] r_texture_address texture_addressing(r_texture_handle tex) const;
//   [[nodiscard]] uvec3 texture_extent(r_texture_handle tex) const;
//   [[nodiscard]] uint32 texture_layers(r_texture_handle tex) const;
//   [[nodiscard]] uint32 texture_levels(r_texture_handle tex) const;
//
// public:
//   [[nodiscard]] r_expected<r_framebuffer_handle> framebuffer_create(
//     const r_framebuffer_descriptor& desc
//   ) noexcept;
//
//   [[nodiscard]] r_framebuffer_handle framebuffer_create(
//     unchecked_t,
//     const r_framebuffer_descriptor& desc
//   );
//
//   void destroy(r_framebuffer_handle fbo) noexcept;
//
//   void framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags);
//   void framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp);
//   void framebuffer_color(r_framebuffer_handle fbo, color4 color);
//
//   [[nodiscard]] r_clear_flag framebuffer_clear(r_framebuffer_handle fbo) const;
//   [[nodiscard]] uvec4 framebuffer_viewport(r_framebuffer_handle fbo) const;
//   [[nodiscard]] color4 framebuffer_color(r_framebuffer_handle fbo) const;
//
// public:
//   [[nodiscard]] r_expected<r_shader_handle> shader_create(
//     const r_shader_descriptor& desc
//   ) noexcept;
//
//   [[nodiscard]] r_shader_handle shader_create(
//     unchecked_t,
//     const r_shader_descriptor& desc
//   );
//
//   void destroy(r_shader_handle shader) noexcept;
//
//   [[nodiscard]] r_shader_type shader_type(r_shader_handle shader) const;
//
// public:
//   [[nodiscard]] r_expected<r_pipeline_handle> pipeline_create(
//     const r_pipeline_descriptor& desc
//   ) noexcept;
//
//   [[nodiscard]] r_pipeline_handle pipeline_create(unchecked_t,
//     const r_pipeline_descriptor& desc
//                                                   );
//
//   void destroy(r_pipeline_handle pipeline) noexcept;
//
//   [[nodiscard]] r_stages_flag pipeline_stages(r_pipeline_handle pipeline) const;
//
//   [[nodiscard]] optional<r_uniform> pipeline_uniform(
//     r_pipeline_handle pipeline,
//    std::string_view name) const noexcept;
//
//   [[nodiscard]] r_uniform pipeline_uniform(
//     unchecked_t,
//     r_pipeline_handle pipeline,
//     std::string_view name) const;
//
// private:
//   void _populate_attachments(
//     r_context_data::framebuffer_store& fb,
//     const r_framebuffer_descriptor& desc,
//     r_framebuffer_handle handle
//   );
//
//   std::unique_ptr<r_context_data::vertex_layout> _copy_pipeline_layout(
//     const r_pipeline_descriptor& desc
//   );
//
// public:
//   // r_api render_api() const { return _ctx->query_meta().api; }
//   std::string_view name_str() const;
//
// protected:
//   weak_ref<r_context_data> _data;
// };
//
// class r_context : public std::unique_ptr<r_context_data>, public r_context_view {
// private:
//   using uptr_base = std::unique_ptr<r_context_data>;
//
// public:
//   r_context(uptr_base data) noexcept;
//
// public:
//   static r_expected<r_context> create(const r_context_params& params) noexcept;
//   static r_context create(unchecked_t, const r_context_params& params);
//
// public:
//   ~r_context() noexcept;
//
//   r_context(const r_context&) = delete;
//   r_context& operator=(const r_context&) = delete;
//
//   r_context(r_context&&) = default;
//   r_context& operator=(r_context&& other) noexcept {
//     r_context_view::operator=(std::move(other));
//     uptr_base::operator=(std::move(other));
//     return *this;
//   }
// };
//
} // namespace ntf
