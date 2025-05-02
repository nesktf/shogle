#include "./meshes.hpp"

#include "../assets/meshes.hpp"

namespace ntf {

r_expected<quad_mesh> quad_mesh::create(r_context_view ctx,
                                        bool inverted_uvs, bool dynamic_storage) {
  r_buffer_flag flags = dynamic_storage ? r_buffer_flag::dynamic_storage : r_buffer_flag::none;
  auto ebo = renderer_buffer::create(ctx,
                                     pnt_indexed_quad_ind,
                                     r_buffer_type::index,
                                     flags);
  if (!ebo) {
    return unexpected{std::move(ebo.error())};
  }
  if (inverted_uvs) {
    auto vbo = renderer_buffer::create(ctx,
                                       pnt_indexed_quad_vert_inv,
                                       r_buffer_type::vertex,
                                       flags);
    if (!vbo) {
      return unexpected{std::move(vbo.error())};
    }
    return quad_mesh{std::move(*vbo), std::move(*ebo), inverted_uvs};
  } else {
    auto vbo = renderer_buffer::create(ctx,
                                       pnt_indexed_quad_vert,
                                       r_buffer_type::vertex,
                                       flags);
    if (!vbo){
      return unexpected{std::move(vbo.error())};
    }
    return quad_mesh{std::move(*vbo), std::move(*ebo), inverted_uvs};
  }
}

r_expected<cube_mesh> cube_mesh::create(r_context_view ctx, bool dynamic_storage) {
  r_buffer_flag flags = dynamic_storage ? r_buffer_flag::dynamic_storage : r_buffer_flag::none;
  auto vbo = renderer_buffer::create(ctx,
                                     pnt_unindexed_cube_vert,
                                     r_buffer_type::vertex,
                                     flags);
  if (!vbo) {
    return unexpected{std::move(vbo.error())};
  }
  return cube_mesh{std::move(*vbo)};
}

} // namespace ntf
