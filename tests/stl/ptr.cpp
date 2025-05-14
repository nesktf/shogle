#include <gtest/gtest.h>

#include "../../src/stl/ptr.hpp"

TEST(stl_ptr, unique_arr_default) {
  ntf::unique_array<int> int_arr;
  ASSERT_FALSE(int_arr.has_data());
  ASSERT_TRUE(int_arr.size() == 0u);
}
