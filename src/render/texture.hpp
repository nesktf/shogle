#pragma once

#include "./common.hpp"

namespace shogle {

struct texture_t_ : public ctx_res_node<texture_t_> {
public:
  texture_t_(context_t ctx, ctx_tex handle_, const ctx_tex_desc& desc) noexcept;

public:
  ctx_tex handle;
  std::atomic<uint32> refcount;
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 levels;
  uint32 layers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(texture_t_);
};

} // namespace shogle
