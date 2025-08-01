#pragma once

#include "./common.hpp"

namespace shogle {

struct buffer_t_ : public ctx_res_node<buffer_t_> {
public:
  buffer_t_(context_t ctx_, ctx_buff handle_, const ctx_buff_desc& desc) noexcept;

public:
  ctx_buff handle;
  buffer_type type;
  buffer_flag flags;
  size_t size;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(buffer_t_);
};

} // namespace shogle
