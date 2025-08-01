#pragma once

#include "./common.hpp"

namespace shogle {

struct framebuffer_t_ : public ctx_res_node<framebuffer_t_> {
// public:
  // enum attachment_state {
  //   ATT_NONE = 0,
  //   ATT_TEX,
  //   ATT_BUFF,
  // };

public:
  framebuffer_t_(context_t ctx_,
                 extent2d extent_, fbo_buffer test_buffer_,
                 const ctx_render_data::fbo_data_t& fdata_) noexcept;

  framebuffer_t_(context_t ctx_, ctx_fbo handle_,
                 extent2d extent_, fbo_buffer test_buffer_,
                 ctx_alloc::uarray_t<fbo_image>&& attachments,
                 const ctx_render_data::fbo_data_t& fdata_) noexcept;

  // framebuffer_t_(context_t ctx_, ctx_fbo handle_,
  //                extent2d extent_, fbo_buffer test_buffer_,
  //                image_format color_buffer_,
  //                const ctx_render_data::fbo_data_t& fdata_) noexcept;

public:
  ctx_fbo handle;
  extent2d extent;
  fbo_buffer test_buffer;
  ctx_alloc::uarray_t<fbo_image> attachments;
  ctx_alloc::vec_t<ctx_render_cmd> cmds;
  ctx_render_data::fbo_data_t fdata;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(framebuffer_t_);
};


} // namespace shogle
