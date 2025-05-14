#include <gtest/gtest.h>

#include "../../src/stl/function.hpp"

namespace {

class funny_functor {
  int _val;

public:
  funny_functor(int val) : _val{val} {}

  int operator()(int a) {
    return _val * a;
  }
};

int fptr(int a) {
  return a*2;
}

int view_fun(ntf::function_view<int()> fun, const int& value) {
  return fun() + value;
}

} // namespace

TEST(stl_function, inplace_function_construct_empty) {
  ntf::inplace_function<int(int)> f;
  ASSERT_TRUE(f.is_empty());

  ntf::inplace_function<int(int)> g = nullptr;
  ASSERT_TRUE(g.is_empty());
}

TEST(stl_function, inplace_function_construct_functor) {
  ntf::inplace_function<int(int)> f = [](int a) {
    return a + 2;
  };
  ASSERT_TRUE(!f.is_empty());
  ASSERT_EQ(8, f(6));
}

TEST(stl_function, inplace_function_construct_function) {
  ntf::inplace_function<int(int)> f = &fptr;
  ASSERT_TRUE(!f.is_empty());
  ASSERT_EQ(16, f(8));
}

TEST(stl_function, inplace_function_construct_in_place) {
  ntf::inplace_function<int(int)> f{std::in_place_type_t<funny_functor>{}, 2};
  ASSERT_TRUE(!f.is_empty());
  ASSERT_EQ(4, f(2));
}

TEST(stl_function, inplace_function_copy) {
  int a = 7;
  ntf::inplace_function<int(int)> f = [a](int b) {
    return a+b;
  };
  ASSERT_TRUE(!f.is_empty());
  const auto fret = f(3);
  ASSERT_EQ(10, fret);

  ntf::inplace_function<int(int)> g = f;
  ASSERT_TRUE(!g.is_empty());
  ASSERT_EQ(fret, g(3));
}

TEST(stl_function, inplace_function_move) {
  int a = 10;
  ntf::inplace_function<int(int)> f = [a](int b) {
    return a*b;
  };
  ASSERT_TRUE(!f.is_empty());
  const auto fret = f(2);
  ASSERT_EQ(20, fret);

  ntf::inplace_function<int(int)> g = std::move(f);
  ASSERT_TRUE(!g.is_empty());
  ASSERT_EQ(fret, g(2));
  EXPECT_TRUE(f.is_empty());
}

TEST(stl_function, inplace_function_assign_empty) {
  ntf::inplace_function<void()> f = []() {};
  ASSERT_TRUE(!f.is_empty());
  f = nullptr;
  ASSERT_TRUE(f.is_empty());
}

TEST(stl_function, inplace_function_assign_lambda) {
  ntf::inplace_function<int(int)> f = [](int) { return 4; };
  ASSERT_TRUE(!f.is_empty());

  f = [](int) { return 2; };
  ASSERT_TRUE(!f.is_empty());
  ASSERT_EQ(2, f(2));
}

TEST(stl_function, inplace_function_assign_ptr) {
  ntf::inplace_function<int(int)> f = [](int) { return 4; };
  ASSERT_TRUE(!f.is_empty());

  f = +[](int) { return 8; };
  ASSERT_TRUE(!f.is_empty());
  ASSERT_EQ(8, f(8));
}

TEST(stl_function, function_view_construct) {
  auto f = []() -> int { return 2; };
  const auto ret = view_fun(f, 2);
  ASSERT_EQ(4, ret);
}
