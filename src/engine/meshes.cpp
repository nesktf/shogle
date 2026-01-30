#include <shogle/scene/meshes.hpp>

#include <shogle/assets/meshes.hpp>

namespace shogle {

render_expect<quad_mesh> quad_mesh::create(context_view ctx, bool inverted_uvs, bool dynamic_storage) {
  buffer_flag flags = dynamic_storage ? buffer_flag::dynamic_storage : buffer_flag::none;
  auto ebo = index_buffer::create(ctx, pnt_indexed_quad_ind, flags);

  if (!ebo) {
    return ntf::unexpected{std::move(ebo.error())};
  }
  if (inverted_uvs) {
    auto vbo = vertex_buffer::create(ctx, pnt_indexed_quad_vert_inv, flags);
    if (!vbo) {
      return ntf::unexpected{std::move(vbo.error())};
    }
    return quad_mesh{std::move(*vbo), std::move(*ebo), inverted_uvs};
  } else {
    auto vbo = vertex_buffer::create(ctx, pnt_indexed_quad_vert, flags);
    if (!vbo){
      return ntf::unexpected{std::move(vbo.error())};
    }
    return quad_mesh{std::move(*vbo), std::move(*ebo), inverted_uvs};
  }
}

render_expect<cube_mesh> cube_mesh::create(context_view ctx, bool dynamic_storage) {
  buffer_flag flags = dynamic_storage ? buffer_flag::dynamic_storage : buffer_flag::none;
  auto vbo = vertex_buffer::create(ctx, pnt_unindexed_cube_vert, flags);
  if (!vbo) {
    return ntf::unexpected{std::move(vbo.error())};
  }
  return cube_mesh{std::move(*vbo)};
}

} // namespace shogle
