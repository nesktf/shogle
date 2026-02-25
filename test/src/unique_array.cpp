#include <catch2/catch_test_macros.hpp>
#include <shogle/util/memory.hpp>

namespace {

struct int_arena {
  size_t pos{0};
  int mem[20];
};

struct int_alloc {
public:
  using value_type = int;
  using size_type = size_t;

public:
  int_alloc(int_arena& arena) : _arena{&arena} {}

public:
  int* allocate(size_t count) {
    int* val = _arena->mem + _arena->pos;
    _arena->pos += count;
    return val;
  }

  void deallocate(int*, size_t) noexcept {}

  bool operator==(const int_alloc&) const noexcept { return true; }

private:
  int_arena* _arena;
};

} // namespace

TEST_CASE("unique array empty construction", "[unique_array]") {
  int_arena arena;
  int_alloc alloc{arena};

  shogle::unique_array<int> arr;
  REQUIRE(arr.empty());

  shogle::unique_array<int> arr_null = nullptr;
  REQUIRE(arr_null.empty());
}

TEST_CASE("unique array construction and allocation", "[unique_array]") {
  const size_t count = 4u;
  const int copy = 10;

  SECTION("Use std::allocator, construct from pointer") {
    std::allocator<int> alloc;
    auto* ptr = alloc.allocate(count);
    for (size_t i = 0; i < count; ++i) {
      new (ptr + i) int{copy};
    }
    shogle::unique_array<int> arr(ptr, count);

    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);

    for (const int i : arr) {
      REQUIRE(i == copy);
    }

    arr.reset();
    REQUIRE(arr.empty());
  }

  SECTION("Use std::allocator, default construct from factory") {
    auto arr = shogle::make_array<int>(count);
    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);

    for (const int n : arr) {
      REQUIRE(n == 0);
    }

    arr.reset();
    REQUIRE(arr.empty());
  }

  SECTION("Use std::allocator, construct from factory using copy") {
    auto arr = shogle::make_array<int>(count, copy);
    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);

    for (const int n : arr) {
      REQUIRE(n == copy);
    }

    arr.reset();
    REQUIRE(arr.empty());
  }

  SECTION("Use std::allocator, construct from factory, uninitialized") {
    std::allocator<int> alloc;
    auto arr = shogle::make_array<int>(shogle::uninitialized, count);
    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);

    arr.reset();
    REQUIRE(arr.empty());
  }
}

TEST_CASE("unique array move operations"
          "[unique_array]") {
  const size_t count = 8u;
  SECTION("Use std::allocator") {
    auto arr = shogle::make_array<int>(shogle::uninitialized, count);
    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);

    auto arr2 = std::move(arr);
    REQUIRE(arr.empty());
    REQUIRE(!arr2.empty());
    REQUIRE(arr2.size() == count);

    arr = std::move(arr2);
    REQUIRE(arr2.empty());
    REQUIRE(!arr.empty());
    REQUIRE(arr.size() == count);
  }
}

TEST_CASE("unique array reset", "[unique_array]") {
  const size_t count1 = 8u;
  const size_t count2 = 4u;
  {
    auto arr1 = shogle::make_array<int>(shogle::uninitialized, count1);
    REQUIRE(!arr1.empty());
    REQUIRE(arr1.size() == count1);

    auto arr2 = shogle::make_array<int>(shogle::uninitialized, count2);
    REQUIRE(!arr2.empty());
    REQUIRE(arr2.size() == count2);

    auto [ptr, sz] = arr2.release();
    REQUIRE(arr2.empty());
    REQUIRE(sz == count2);

    arr1.assign(ptr, sz);
    REQUIRE(arr1.size() == count2);
  }
}
