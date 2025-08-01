#pragma once

#include "./common.hpp"

namespace shogle {

struct shader_t_ : public ctx_res_node<shader_t_> {
public:
  shader_t_(context_t ctx_, ctx_shad handle_, const ctx_shad_desc& desc) noexcept;

public:
  ctx_shad handle;
  shader_type type;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(shader_t_);
};

struct pipeline_t_ : public ctx_res_node<pipeline_t_> {
public:
  pipeline_t_(context_t ctx_, ctx_pip handle_,
              stages_flag stages_, primitive_mode primitive_, polygon_mode poly_mode_,
              ctx_alloc::uarray_t<attribute_binding>&& layout,
              unif_meta_vec&& unifs_) noexcept;

public:
  ctx_pip handle;
  stages_flag stages;
  primitive_mode primitive;
  polygon_mode poly_mode;
  ctx_alloc::uarray_t<attribute_binding> layout;
  unif_meta_vec unifs;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(pipeline_t_);
};

} // namespace shogle
