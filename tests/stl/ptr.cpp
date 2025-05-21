#include <gtest/gtest.h>

#include "../../src/stl/ptr.hpp"

namespace {

struct int_arena {
  size_t pos{0};
  int mem[20];
};

struct int_alloc {
public:
  using value_type = int;

public:
  int_alloc(int_arena& arena) :
    _arena{&arena} {}

public:
  int* allocate(size_t count) {
    int* val = _arena->mem+_arena->pos;
    _arena->pos += count;
    return val;
  }

  void deallocate(int*, size_t) {}

private:
  int_arena* _arena;
};

using int_alloc_del = ntf::allocator_delete<int, int_alloc>;

} // namespace

TEST(stl_ptr, unique_array_empty) {
  int_arena arena;
  int_alloc alloc{arena};

  ntf::unique_array<int> arr;
  ASSERT_TRUE(arr.empty());

  ntf::unique_array<int> arr_null = nullptr;
  ASSERT_TRUE(arr_null.empty());

  ntf::unique_array<int, int_alloc_del> arr_alloc{alloc};
  ASSERT_TRUE(arr_alloc.empty());
}

TEST(stl_ptr, unique_array_alloc) {
  const size_t count = 4u;
  const int copy = 10;
  {
    auto* ptr = std::allocator<int>{}.allocate(count);
    for (size_t i = 0; i < count; ++i) {
      new (ptr+i) int{copy};
    }

    ntf::unique_array<int> arr{count, ptr};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int i : arr){
      ASSERT_TRUE(i == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    auto arr = ntf::unique_array<int>::from_allocator(count, copy);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int n : arr) {
      ASSERT_TRUE(n == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    auto arr = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    auto* ptr = alloc.allocate(count);
    for (size_t i = 0; i < count; ++i) {
      new (ptr+i) int{copy};
    }

    ntf::unique_array<int, int_alloc_del> arr{count, ptr, alloc};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int i : arr){
      ASSERT_TRUE(i == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    auto arr = ntf::unique_array<int>::from_allocator(count, copy, alloc);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int n : arr) {
      ASSERT_TRUE(n == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    auto arr = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count, alloc);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
}

TEST(stl_ptr, unique_array_new) {
  const size_t count = 8u;
  const int copy = 10;
  auto* ptr = new int[8u];
  for (size_t i = 0; i < count; ++i){
    ptr[i] = copy;
  }

  ntf::unique_array<int, std::default_delete<int[]>> arr{count, ptr};
  ASSERT_TRUE(!arr.empty());
  ASSERT_TRUE(arr.size() == count);

  for (const int i : arr){
    ASSERT_TRUE(i == copy);
  }

  arr.reset();
  ASSERT_TRUE(arr.empty());
}

TEST(stl_ptr, unique_array_alloc_inplace) {
  const size_t count = 8u;
  const int copy = 10;
  {
    // Allocates using std::allocator
    ntf::unique_array<int> arr{count, copy};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int n : arr) {
      ASSERT_TRUE(n == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    ntf::unique_array<int, int_alloc_del> arr{count, copy, alloc};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    for (const int n : arr) {
      ASSERT_TRUE(n == copy);
    }

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    // Allocates using std::allocator
    ntf::unique_array<int> arr{ntf::uninitialized, count};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    ntf::unique_array<int, int_alloc_del> arr{ntf::uninitialized, count, alloc};
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    arr.reset();
    ASSERT_TRUE(arr.empty());
  }
}

TEST(stl_ptr, unique_array_move) {
  const size_t count = 8u;
  {
    auto arr = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    auto arr2 = std::move(arr);
    ASSERT_TRUE(arr.empty());
    ASSERT_TRUE(!arr2.empty());
    ASSERT_TRUE(arr2.size() == count);

    arr = std::move(arr2);
    ASSERT_TRUE(arr2.empty());
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);
  }
  {
    int_arena arena;
    int_alloc alloc{arena};

    auto arr = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count, alloc);
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);

    auto arr2 = std::move(arr);
    ASSERT_TRUE(arr.empty());
    ASSERT_TRUE(!arr2.empty());
    ASSERT_TRUE(arr2.size() == count);

    arr = std::move(arr2);
    ASSERT_TRUE(arr2.empty());
    ASSERT_TRUE(!arr.empty());
    ASSERT_TRUE(arr.size() == count);
  }
}

TEST(stl_ptr, unique_array_reset) {
  const size_t count1 = 8u;
  const size_t count2 = 4u;;
  {
    auto arr1 = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count1);
    ASSERT_TRUE(!arr1.empty());
    ASSERT_TRUE(arr1.size() == count1);

    auto arr2 = ntf::unique_array<int>::from_allocator(ntf::uninitialized, count2);
    ASSERT_TRUE(!arr2.empty());
    ASSERT_TRUE(arr2.size() == count2);

    auto [ptr, sz] = arr2.release();
    ASSERT_TRUE(arr2.empty());
    ASSERT_TRUE(sz == count2);

    arr1.reset(sz, ptr);
    ASSERT_TRUE(arr1.size() == count2);
  }
}
