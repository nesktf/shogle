#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <shogle/util/expected.hpp>

using namespace shogle::numdefs;

enum class network_error { timeout, disconnected, unknown };

struct nontrivial_thing {
  static inline int alive_count = 0;

  int id;

  nontrivial_thing(int i) : id(i) { ++alive_count; }

  nontrivial_thing(nontrivial_thing&& other) noexcept : id(other.id) { ++alive_count; }

  nontrivial_thing(const nontrivial_thing& other) noexcept : id(other.id) { ++alive_count; }

  ~nontrivial_thing() noexcept { --alive_count; }

  nontrivial_thing& operator=(const nontrivial_thing&) = delete; // we don't care about this
  nontrivial_thing& operator=(nontrivial_thing&&) = delete;      // we don't care about this

  bool operator==(const nontrivial_thing& other) const { return id == other.id; }
};

TEST_CASE("basic expected construction", "[expected]") {
  SECTION("default construction") {
    shogle::expected<int, network_error> exp;
    REQUIRE(exp.has_value());
    REQUIRE(static_cast<bool>(exp));
    REQUIRE_FALSE(exp.has_error());
    REQUIRE(*exp == 0);
    REQUIRE_NOTHROW(exp.value() == 0);
    REQUIRE(exp.value_or(10) == 0);
    REQUIRE(exp.error_or(network_error::unknown) == network_error::unknown);

    shogle::expected<void, network_error> exp_void;
    REQUIRE(exp_void.has_value());
    REQUIRE(static_cast<bool>(exp_void));
    REQUIRE_FALSE(exp_void.has_error());
  }

  SECTION("value construction with forwarding value") {
    shogle::expected<int, network_error> exp_rvalue(42);
    REQUIRE(exp_rvalue.has_value());
    REQUIRE(static_cast<bool>(exp_rvalue));
    REQUIRE_FALSE(exp_rvalue.has_error());
    REQUIRE(*exp_rvalue == 42);
    REQUIRE_NOTHROW(exp_rvalue.value() == 42);
    REQUIRE(exp_rvalue.value_or(0) == 42);
    REQUIRE(exp_rvalue.error_or(network_error::unknown) == network_error::unknown);

    const int val = 40;
    shogle::expected<int, network_error> exp_lvalue(val);
    REQUIRE(exp_lvalue.has_value());
    REQUIRE(static_cast<bool>(exp_lvalue));
    REQUIRE_FALSE(exp_lvalue.has_error());
    REQUIRE(*exp_lvalue == 40);
    REQUIRE_NOTHROW(exp_rvalue.value() == 40);
    REQUIRE(exp_lvalue.value_or(0) == 40);
    REQUIRE(exp_lvalue.error_or(network_error::unknown) == network_error::unknown);
  }

  SECTION("value in place construction") {
    shogle::expected<int, network_error> exp(shogle::in_place, 42);
    REQUIRE(exp.has_value());
    REQUIRE(static_cast<bool>(exp));
    REQUIRE(*exp == 42);
    REQUIRE(exp.value() == 42);
    REQUIRE(exp.value_or(0) == 42);
    REQUIRE(exp.error_or(network_error::unknown) == network_error::unknown);

    shogle::expected<void, network_error> exp_void(shogle::in_place);
    REQUIRE(exp_void.has_value());
    REQUIRE(static_cast<bool>(exp_void));
    REQUIRE_FALSE(exp_void.has_error());
  }

  SECTION("error construction with forwarding unexpected") {
    shogle::expected<int, network_error> exp_rvalue =
      shogle::unexpected<network_error>(network_error::timeout);
    REQUIRE_FALSE(exp_rvalue.has_value());
    REQUIRE_FALSE(static_cast<bool>(exp_rvalue));
    REQUIRE_THROWS_AS(exp_rvalue.value(), shogle::bad_expected_access<network_error>);
    REQUIRE(exp_rvalue.has_error());
    REQUIRE(exp_rvalue.error() == network_error::timeout);
    REQUIRE(exp_rvalue.value_or(100) == 100);
    REQUIRE(exp_rvalue.error_or(network_error::unknown) == network_error::timeout);

    const shogle::unexpected<network_error> unex(network_error::disconnected);
    shogle::expected<int, network_error> exp_lvalue(unex);
    REQUIRE_FALSE(exp_lvalue.has_value());
    REQUIRE_FALSE(static_cast<bool>(exp_lvalue));
    REQUIRE_THROWS_AS(exp_lvalue.value(), shogle::bad_expected_access<network_error>);
    REQUIRE(exp_lvalue.has_error());
    REQUIRE(exp_lvalue.error() == network_error::disconnected);
    REQUIRE(exp_lvalue.value_or(100) == 100);
    REQUIRE(exp_lvalue.error_or(network_error::unknown) == network_error::disconnected);

    shogle::expected<void, network_error> exp_void_rvalue =
      shogle::unexpected<network_error>(network_error::timeout);
    REQUIRE_FALSE(exp_void_rvalue.has_value());
    REQUIRE_FALSE(static_cast<bool>(exp_void_rvalue));
    REQUIRE_THROWS_AS(exp_void_rvalue.value(), shogle::bad_expected_access<network_error>);
    REQUIRE(exp_void_rvalue.has_error());
    REQUIRE(exp_void_rvalue.error() == network_error::timeout);
    REQUIRE(exp_void_rvalue.error_or(network_error::unknown) == network_error::timeout);

    shogle::expected<void, network_error> exp_void_lvalue(unex);
    REQUIRE_FALSE(exp_void_lvalue.has_value());
    REQUIRE_FALSE(static_cast<bool>(exp_void_lvalue));
    REQUIRE_THROWS_AS(exp_void_lvalue.value(), shogle::bad_expected_access<network_error>);
    REQUIRE(exp_void_lvalue.has_error());
    REQUIRE(exp_void_lvalue.error() == network_error::disconnected);
    REQUIRE(exp_void_lvalue.error_or(network_error::unknown) == network_error::disconnected);
  }
}

TEST_CASE("expected non-void trivial move semantics", "[expected]") {
  static_assert(std::is_trivially_destructible_v<shogle::expected<int, network_error>>);

  shogle::expected<int, network_error> exp_valid(20);
  REQUIRE(exp_valid.has_value());
  REQUIRE_FALSE(exp_valid.has_error());
  REQUIRE_NOTHROW(exp_valid.value() == 20);

  shogle::expected<int, network_error> exp_invalid(shogle::unexpect, network_error::unknown);
  REQUIRE_FALSE(exp_invalid.has_value());
  REQUIRE(exp_invalid.has_error());
  REQUIRE(exp_invalid.error() == network_error::unknown);

  SECTION("copy construction") {
    static_assert(std::is_trivially_copy_constructible_v<shogle::expected<int, network_error>>);

    auto copy_valid = exp_valid;
    REQUIRE(copy_valid.has_value());
    REQUIRE_FALSE(copy_valid.has_error());
    REQUIRE_NOTHROW(copy_valid.value() == 20);
    REQUIRE(copy_valid == exp_valid);

    auto copy_invalid = exp_invalid;
    REQUIRE_FALSE(copy_invalid.has_value());
    REQUIRE(copy_invalid.has_error());
    REQUIRE(copy_invalid.error() == network_error::unknown);
  }

  SECTION("move construction") {
    static_assert(std::is_trivially_move_constructible_v<shogle::expected<int, network_error>>);

    auto moved_valid = std::move(exp_valid);
    REQUIRE(moved_valid.has_value());
    REQUIRE_FALSE(moved_valid.has_error());
    REQUIRE_NOTHROW(moved_valid.value() == exp_valid.value());
    REQUIRE(moved_valid == exp_valid);

    auto moved_invalid = std::move(exp_invalid);
    REQUIRE_FALSE(moved_invalid.has_value());
    REQUIRE(moved_invalid.has_error());
    REQUIRE(moved_invalid.error() == exp_invalid.error());
  }

  SECTION("copy assignment") {
    // Not actually trivial lmao
    static_assert(!std::is_trivially_copy_assignable_v<shogle::expected<int, network_error>>);

    shogle::expected<int, network_error> exp0(20);
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE_NOTHROW(exp0.value() == 20);

    exp0 = exp_valid;
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE_NOTHROW(exp0.value() == exp_valid.value());
    REQUIRE(exp0 == exp_valid);

    shogle::expected<int, network_error> exp1(40);
    REQUIRE(exp1.has_value());
    REQUIRE_FALSE(exp1.has_error());
    REQUIRE_NOTHROW(exp1.value() == 40);

    exp1 = exp_invalid;
    REQUIRE_FALSE(exp1.has_value());
    REQUIRE(exp1.has_error());
    REQUIRE(exp1.error() == exp_invalid.error());

    shogle::expected<int, network_error> exp2(shogle::unexpect, network_error::timeout);
    REQUIRE_FALSE(exp2.has_value());
    REQUIRE(exp2.has_error());
    REQUIRE(exp2.error() == network_error::timeout);

    exp2 = exp_valid;
    REQUIRE(exp2.has_value());
    REQUIRE_FALSE(exp2.has_error());
    REQUIRE_NOTHROW(exp2.value() == exp_valid.value());
    REQUIRE(exp2 == exp_valid);

    shogle::expected<int, network_error> exp3(shogle::unexpect, network_error::disconnected);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == network_error::disconnected);

    exp3 = exp_invalid;
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == exp_invalid.error());
  }

  SECTION("move assignment") {
    // Not actually trivial lmao
    static_assert(!std::is_trivially_move_assignable_v<shogle::expected<int, network_error>>);

    shogle::expected<int, network_error> exp0(20);
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE_NOTHROW(exp0.value() == 20);

    exp0 = std::move(exp_valid);
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE_NOTHROW(exp0.value() == exp_valid.value());
    REQUIRE(exp0 == exp_valid);

    shogle::expected<int, network_error> exp1(40);
    REQUIRE(exp1.has_value());
    REQUIRE_FALSE(exp1.has_error());
    REQUIRE_NOTHROW(exp1.value() == 40);

    exp1 = std::move(exp_invalid);
    REQUIRE_FALSE(exp1.has_value());
    REQUIRE(exp1.has_error());
    REQUIRE(exp1.error() == exp_invalid.error());

    shogle::expected<int, network_error> exp2(shogle::unexpect, network_error::timeout);
    REQUIRE_FALSE(exp2.has_value());
    REQUIRE(exp2.has_error());
    REQUIRE(exp2.error() == network_error::timeout);

    exp2 = std::move(exp_valid);
    REQUIRE(exp2.has_value());
    REQUIRE_FALSE(exp2.has_error());
    REQUIRE_NOTHROW(exp2.value() == exp_valid.value());
    REQUIRE(exp2 == exp_valid);

    shogle::expected<int, network_error> exp3(shogle::unexpect, network_error::disconnected);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == network_error::disconnected);

    exp3 = std::move(exp_invalid);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == exp_invalid.error());
  }
}

TEST_CASE("expected non-trivial move semantics", "[expected]") {
  static_assert(
    !std::is_trivially_destructible_v<shogle::expected<nontrivial_thing, network_error>>);
  static_assert(!std::is_trivially_destructible_v<shogle::expected<int, nontrivial_thing>>);

  nontrivial_thing::alive_count = 0;

  shogle::expected<nontrivial_thing, network_error> expval_valid(shogle::in_place, 20);
  REQUIRE(expval_valid.has_value());
  REQUIRE_FALSE(expval_valid.has_error());
  REQUIRE_NOTHROW(expval_valid.value().id == 20);
  REQUIRE(nontrivial_thing::alive_count == 1);

  shogle::expected<nontrivial_thing, network_error> expval_invalid(shogle::unexpect,
                                                                   network_error::unknown);
  REQUIRE_FALSE(expval_invalid.has_value());
  REQUIRE(expval_invalid.has_error());
  REQUIRE(expval_invalid.error() == network_error::unknown);
  REQUIRE(nontrivial_thing::alive_count == 1);

  shogle::expected<int, nontrivial_thing> experr_valid(20);
  REQUIRE(experr_valid.has_value());
  REQUIRE_FALSE(experr_valid.has_error());
  REQUIRE_NOTHROW(experr_valid.value() == 20);
  REQUIRE(nontrivial_thing::alive_count == 1);

  shogle::expected<int, nontrivial_thing> experr_invalid(shogle::unexpect, 40);
  REQUIRE_FALSE(experr_invalid.has_value());
  REQUIRE(experr_invalid.has_error());
  REQUIRE(experr_invalid.error().id == 40);
  REQUIRE(nontrivial_thing::alive_count == 2);

  SECTION("copy construction") {
    static_assert(
      !std::is_trivially_copy_constructible_v<shogle::expected<nontrivial_thing, network_error>>);
    static_assert(
      !std::is_trivially_copy_constructible_v<shogle::expected<int, nontrivial_thing>>);

    auto copy_valid0 = expval_valid;
    REQUIRE(copy_valid0.has_value());
    REQUIRE_FALSE(copy_valid0.has_error());
    REQUIRE_NOTHROW(copy_valid0.value().id == 20);
    REQUIRE(copy_valid0 == expval_valid);

    auto copy_invalid0 = expval_invalid;
    REQUIRE_FALSE(copy_invalid0.has_value());
    REQUIRE(copy_invalid0.has_error());
    REQUIRE(copy_invalid0.error() == network_error::unknown);

    auto copy_valid1 = experr_valid;
    REQUIRE(copy_valid1.has_value());
    REQUIRE_FALSE(copy_valid1.has_error());
    REQUIRE_NOTHROW(copy_valid1.value() == 20);
    REQUIRE(copy_valid1 == experr_valid);

    auto copy_invalid1 = experr_invalid;
    REQUIRE_FALSE(copy_invalid1.has_value());
    REQUIRE(copy_invalid1.has_error());
    REQUIRE(copy_invalid1.error().id == 40);

    REQUIRE(nontrivial_thing::alive_count == 4);
  }

  SECTION("move construction") {
    static_assert(
      !std::is_trivially_move_constructible_v<shogle::expected<nontrivial_thing, network_error>>);
    static_assert(
      !std::is_trivially_move_constructible_v<shogle::expected<int, nontrivial_thing>>);

    auto copy_valid0 = std::move(expval_valid);
    REQUIRE(copy_valid0.has_value());
    REQUIRE_FALSE(copy_valid0.has_error());
    REQUIRE_NOTHROW(copy_valid0.value().id == 20);
    REQUIRE(copy_valid0 == expval_valid);

    auto copy_invalid0 = std::move(expval_invalid);
    REQUIRE_FALSE(copy_invalid0.has_value());
    REQUIRE(copy_invalid0.has_error());
    REQUIRE(copy_invalid0.error() == network_error::unknown);

    auto copy_valid1 = std::move(experr_valid);
    REQUIRE(copy_valid1.has_value());
    REQUIRE_FALSE(copy_valid1.has_error());
    REQUIRE_NOTHROW(copy_valid1.value() == 20);
    REQUIRE(copy_valid1 == experr_valid);

    auto copy_invalid1 = std::move(experr_invalid);
    REQUIRE_FALSE(copy_invalid1.has_value());
    REQUIRE(copy_invalid1.has_error());
    REQUIRE(copy_invalid1.error().id == 40);

    REQUIRE(nontrivial_thing::alive_count == 4);
  }

  SECTION("copy assignment") {
    static_assert(
      !std::is_trivially_copy_assignable_v<shogle::expected<nontrivial_thing, network_error>>);
    static_assert(!std::is_trivially_copy_assignable_v<shogle::expected<int, nontrivial_thing>>);

    shogle::expected<nontrivial_thing, network_error> expval0(shogle::in_place, 40);
    REQUIRE(expval0.has_value());
    REQUIRE_FALSE(expval0.has_error());
    REQUIRE_NOTHROW(expval0.value().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 3);

    expval0 = expval_valid;
    REQUIRE(expval0.has_value());
    REQUIRE_FALSE(expval0.has_error());
    REQUIRE_NOTHROW(expval0.value() == expval_valid.value());
    REQUIRE(expval0 == expval_valid);
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<nontrivial_thing, network_error> expval1(shogle::in_place, 40);
    REQUIRE(expval1.has_value());
    REQUIRE_FALSE(expval1.has_error());
    REQUIRE_NOTHROW(expval1.value().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 4);

    expval1 = expval_invalid;
    REQUIRE_FALSE(expval1.has_value());
    REQUIRE(expval1.has_error());
    REQUIRE(expval1.error() == expval_invalid.error());
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<int, nontrivial_thing> experr0(60);
    REQUIRE(experr0.has_value());
    REQUIRE_FALSE(experr0.has_error());
    REQUIRE_NOTHROW(experr0.value() == 60);
    REQUIRE(nontrivial_thing::alive_count == 3);

    experr0 = experr_valid;
    REQUIRE(experr0.has_value());
    REQUIRE_FALSE(experr0.has_error());
    REQUIRE_NOTHROW(experr0.value() == experr_valid.value());
    REQUIRE(experr0 == experr_valid);
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<int, nontrivial_thing> experr1(shogle::unexpect, 50);
    REQUIRE_FALSE(experr_invalid.has_value());
    REQUIRE(experr1.has_error());
    REQUIRE(experr1.error().id == 50);
    REQUIRE(nontrivial_thing::alive_count == 4);

    experr1 = experr_invalid;
    REQUIRE_FALSE(experr1.has_value());
    REQUIRE(experr1.has_error());
    REQUIRE(experr1.error() == experr_invalid.error());
    REQUIRE(nontrivial_thing::alive_count == 4);
  }

  SECTION("move assignment") {
    static_assert(
      !std::is_trivially_move_assignable_v<shogle::expected<nontrivial_thing, network_error>>);
    static_assert(!std::is_trivially_move_assignable_v<shogle::expected<int, nontrivial_thing>>);

    shogle::expected<nontrivial_thing, network_error> expval0(shogle::in_place, 40);
    REQUIRE(expval0.has_value());
    REQUIRE_FALSE(expval0.has_error());
    REQUIRE_NOTHROW(expval0.value().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 3);

    expval0 = std::move(expval_valid);
    REQUIRE(expval0.has_value());
    REQUIRE_FALSE(expval0.has_error());
    REQUIRE_NOTHROW(expval0.value() == expval_valid.value());
    REQUIRE(expval0 == expval_valid);
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<nontrivial_thing, network_error> expval1(shogle::in_place, 40);
    REQUIRE(expval1.has_value());
    REQUIRE_FALSE(expval1.has_error());
    REQUIRE_NOTHROW(expval1.value().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 4);

    expval1 = std::move(expval_invalid);
    REQUIRE_FALSE(expval1.has_value());
    REQUIRE(expval1.has_error());
    REQUIRE(expval1.error() == expval_invalid.error());
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<int, nontrivial_thing> experr0(60);
    REQUIRE(experr0.has_value());
    REQUIRE_FALSE(experr0.has_error());
    REQUIRE_NOTHROW(experr0.value() == 60);
    REQUIRE(nontrivial_thing::alive_count == 3);

    experr0 = std::move(experr_valid);
    REQUIRE(experr0.has_value());
    REQUIRE_FALSE(experr0.has_error());
    REQUIRE_NOTHROW(experr0.value() == experr_valid.value());
    REQUIRE(experr0 == experr_valid);
    REQUIRE(nontrivial_thing::alive_count == 3);

    shogle::expected<int, nontrivial_thing> experr1(shogle::unexpect, 50);
    REQUIRE_FALSE(experr_invalid.has_value());
    REQUIRE(experr1.has_error());
    REQUIRE(experr1.error().id == 50);
    REQUIRE(nontrivial_thing::alive_count == 4);

    experr1 = std::move(experr_invalid);
    REQUIRE_FALSE(experr1.has_value());
    REQUIRE(experr1.has_error());
    REQUIRE(experr1.error() == experr_invalid.error());
    REQUIRE(nontrivial_thing::alive_count == 4);
  }
}

TEST_CASE("expected void trivial move semantics", "[expected]") {
  static_assert(std::is_trivially_destructible_v<shogle::expected<void, network_error>>);

  shogle::expected<void, network_error> exp_valid;
  REQUIRE(exp_valid.has_value());
  REQUIRE_FALSE(exp_valid.has_error());

  shogle::expected<void, network_error> exp_invalid(shogle::unexpect, network_error::unknown);
  REQUIRE_FALSE(exp_invalid.has_value());
  REQUIRE(exp_invalid.has_error());
  REQUIRE(exp_invalid.error() == network_error::unknown);

  SECTION("copy construction") {
    static_assert(std::is_trivially_copy_constructible_v<shogle::expected<void, network_error>>);

    auto copy_valid = exp_valid;
    REQUIRE(copy_valid.has_value());
    REQUIRE_FALSE(copy_valid.has_error());
    REQUIRE(copy_valid == exp_valid);

    auto copy_invalid = exp_invalid;
    REQUIRE_FALSE(copy_invalid.has_value());
    REQUIRE(copy_invalid.has_error());
    REQUIRE(copy_invalid.error() == network_error::unknown);
  }

  SECTION("move construction") {
    static_assert(std::is_trivially_move_constructible_v<shogle::expected<void, network_error>>);

    auto moved_valid = std::move(exp_valid);
    REQUIRE(moved_valid.has_value());
    REQUIRE_FALSE(moved_valid.has_error());
    REQUIRE(moved_valid == exp_valid);

    auto moved_invalid = std::move(exp_invalid);
    REQUIRE_FALSE(moved_invalid.has_value());
    REQUIRE(moved_invalid.has_error());
    REQUIRE(moved_invalid.error() == network_error::unknown);
  }

  SECTION("copy assignment") {
    // Not actually trivial lmao
    static_assert(!std::is_trivially_copy_assignable_v<shogle::expected<void, network_error>>);

    shogle::expected<void, network_error> exp0;
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());

    exp0 = exp_valid;
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE(exp0 == exp_valid);

    shogle::expected<void, network_error> exp1;
    REQUIRE(exp1.has_value());
    REQUIRE_FALSE(exp1.has_error());

    exp1 = exp_invalid;
    REQUIRE_FALSE(exp1.has_value());
    REQUIRE(exp1.has_error());
    REQUIRE(exp1.error() == exp_invalid.error());

    shogle::expected<void, network_error> exp2(shogle::unexpect, network_error::timeout);
    REQUIRE_FALSE(exp2.has_value());
    REQUIRE(exp2.has_error());
    REQUIRE(exp2.error() == network_error::timeout);

    exp2 = exp_valid;
    REQUIRE(exp2.has_value());
    REQUIRE_FALSE(exp2.has_error());
    REQUIRE(exp2 == exp_valid);

    shogle::expected<void, network_error> exp3(shogle::unexpect, network_error::disconnected);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == network_error::disconnected);

    exp3 = exp_invalid;
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == exp_invalid.error());
  }

  SECTION("move assignment") {
    // Not actually trivial lmao
    static_assert(!std::is_trivially_move_assignable_v<shogle::expected<void, network_error>>);

    shogle::expected<void, network_error> exp0;
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());

    exp0 = std::move(exp_valid);
    REQUIRE(exp0.has_value());
    REQUIRE_FALSE(exp0.has_error());
    REQUIRE(exp0 == exp_valid);

    shogle::expected<void, network_error> exp1;
    REQUIRE(exp1.has_value());
    REQUIRE_FALSE(exp1.has_error());

    exp1 = std::move(exp_invalid);
    REQUIRE_FALSE(exp1.has_value());
    REQUIRE(exp1.has_error());
    REQUIRE(exp1.error() == exp_invalid.error());

    shogle::expected<void, network_error> exp2(shogle::unexpect, network_error::timeout);
    REQUIRE_FALSE(exp2.has_value());
    REQUIRE(exp2.has_error());
    REQUIRE(exp2.error() == network_error::timeout);

    exp2 = std::move(exp_valid);
    REQUIRE(exp2.has_value());
    REQUIRE_FALSE(exp2.has_error());
    REQUIRE(exp2 == exp_valid);

    shogle::expected<void, network_error> exp3(shogle::unexpect, network_error::disconnected);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == network_error::disconnected);

    exp3 = std::move(exp_invalid);
    REQUIRE_FALSE(exp3.has_value());
    REQUIRE(exp3.has_error());
    REQUIRE(exp3.error() == exp_invalid.error());
  }
}

TEST_CASE("expected void non-trivial move semantics", "[expected]") {
  static_assert(!std::is_trivially_destructible_v<shogle::expected<void, nontrivial_thing>>);

  nontrivial_thing::alive_count = 0;

  shogle::expected<void, nontrivial_thing> exp_valid;
  REQUIRE(exp_valid.has_value());
  REQUIRE_FALSE(exp_valid.has_error());
  REQUIRE(nontrivial_thing::alive_count == 0);

  shogle::expected<void, nontrivial_thing> exp_invalid(shogle::unexpect, 20);
  REQUIRE_FALSE(exp_invalid.has_value());
  REQUIRE(exp_invalid.has_error());
  REQUIRE(exp_invalid.error().id == 20);
  REQUIRE(nontrivial_thing::alive_count == 1);

  // TODO
  SECTION("copy construction") {
    static_assert(
      !std::is_trivially_copy_constructible_v<shogle::expected<void, nontrivial_thing>>);
  }
  SECTION("move construction") {
    static_assert(
      !std::is_trivially_move_constructible_v<shogle::expected<void, nontrivial_thing>>);
  }
  SECTION("copy assignment") {
    static_assert(!std::is_trivially_copy_assignable_v<shogle::expected<void, nontrivial_thing>>);
  }
  SECTION("move assignment") {
    static_assert(!std::is_trivially_move_assignable_v<shogle::expected<void, nontrivial_thing>>);
  }
}

TEST_CASE("expected emplacing", "[expected]") {
  nontrivial_thing::alive_count = 0;

  SECTION("emplace non-void") {
    shogle::expected<int, nontrivial_thing> exp(shogle::in_place, 341);
    REQUIRE(exp.has_value());
    REQUIRE_FALSE(exp.has_error());
    REQUIRE_NOTHROW(exp.value() == 341);
    REQUIRE(nontrivial_thing::alive_count == 0);

    shogle::expected<int, nontrivial_thing> exp_err(shogle::unexpect, 40);
    REQUIRE_FALSE(exp_err.has_value());
    REQUIRE(exp_err.has_error());
    REQUIRE(exp_err.error().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 1);

    exp.emplace(31);
    REQUIRE(exp.has_value());
    REQUIRE_FALSE(exp.has_error());
    REQUIRE_NOTHROW(exp.value() == 31);
    REQUIRE(nontrivial_thing::alive_count == 1);

    exp.emplace_error(32);
    REQUIRE_FALSE(exp.has_value());
    REQUIRE(exp.has_error());
    REQUIRE(exp.error().id == 32);
    REQUIRE(nontrivial_thing::alive_count == 2);

    exp_err.emplace_error(50);
    REQUIRE_FALSE(exp_err.has_value());
    REQUIRE(exp_err.has_error());
    REQUIRE(exp_err.error().id == 50);
    REQUIRE(nontrivial_thing::alive_count == 2);

    exp_err.emplace(451);
    REQUIRE_FALSE(exp.has_value());
    REQUIRE(exp.has_error());
    REQUIRE_NOTHROW(exp_err.value() == 451);
    REQUIRE(nontrivial_thing::alive_count == 1);
  }

  SECTION("emplace void") {
    shogle::expected<void, nontrivial_thing> exp;
    REQUIRE(exp.has_value());
    REQUIRE_FALSE(exp.has_error());
    REQUIRE(nontrivial_thing::alive_count == 0);

    shogle::expected<void, nontrivial_thing> exp_err(shogle::unexpect, 40);
    REQUIRE_FALSE(exp_err.has_value());
    REQUIRE(exp_err.has_error());
    REQUIRE(exp_err.error().id == 40);
    REQUIRE(nontrivial_thing::alive_count == 1);

    exp.emplace();
    REQUIRE(exp.has_value());
    REQUIRE_FALSE(exp.has_error());
    REQUIRE(nontrivial_thing::alive_count == 1);

    exp.emplace_error(32);
    REQUIRE_FALSE(exp.has_value());
    REQUIRE(exp.has_error());
    REQUIRE(exp.error().id == 32);
    REQUIRE(nontrivial_thing::alive_count == 2);

    exp_err.emplace_error(50);
    REQUIRE_FALSE(exp_err.has_value());
    REQUIRE(exp_err.has_error());
    REQUIRE(exp_err.error().id == 50);
    REQUIRE(nontrivial_thing::alive_count == 2);

    exp_err.emplace();
    REQUIRE_FALSE(exp.has_value());
    REQUIRE(exp.has_error());
    REQUIRE(nontrivial_thing::alive_count == 1);
  }
}

TEST_CASE("expected monadic operations", "[expected]") {
  const auto safe_divide = [](int a, int b) -> shogle::expected<int, std::string> {
    if (b == 0) {
      return shogle::unexpected<std::string>("Division by zero");
    }
    return a / b;
  };
  const auto num_to_str = [](int val) {
    return "Number: " + std::to_string(val);
  };

  SECTION("transform") {
    const shogle::expected<int, std::string> exp_lvalue(10);
    const auto res_lvalue = exp_lvalue.transform(num_to_str);
    REQUIRE(res_lvalue.has_value());
    REQUIRE(res_lvalue.value() == "Number: 10");

    const shogle::expected<int, std::string> exp_lvalue_err =
      shogle::unexpected<std::string>("Error");
    const auto err_lvalue = exp_lvalue_err.transform(num_to_str);
    REQUIRE_FALSE(err_lvalue.has_value());
    REQUIRE(err_lvalue.error() == "Error");

    const auto res_rvalue = shogle::expected<int, std::string>(10).transform(num_to_str);
    REQUIRE(res_rvalue.has_value());
    REQUIRE(res_rvalue.value() == "Number: 10");

    const auto err_rvalue =
      shogle::expected<int, std::string>(shogle::unexpected<std::string>("Error"))
        .transform(num_to_str);
    REQUIRE_FALSE(err_rvalue.has_value());
    REQUIRE(err_rvalue.error() == "Error");

    int value = 0;
    const shogle::expected<void, std::string> exp_void_lvalue;
    const auto res_void_lvalue = exp_void_lvalue.transform([&]() {
      value = 2;
      return value;
    });
    REQUIRE(res_void_lvalue.has_value());
    REQUIRE_NOTHROW(res_void_lvalue.value() == 2);
    REQUIRE(value == 2);

    value = 0;
    const shogle::expected<void, std::string> exp_void_lvalue_err =
      shogle::unexpected<std::string>("Error");
    const auto err_void_lvalue = exp_void_lvalue_err.transform([&]() {
      value = 5;
      return value;
    });
    REQUIRE_FALSE(err_void_lvalue.has_value());
    REQUIRE(value == 0);

    value = 0;
    const auto res_void_rvalue = shogle::expected<void, std::string>().transform([&]() {
      value = 4;
      return value;
    });
    REQUIRE(res_void_rvalue.has_value());
    REQUIRE_NOTHROW(res_void_rvalue.value() == 4);
    REQUIRE(value == 4);

    value = 0;
    const auto err_void_rvalue =
      shogle::expected<void, std::string>(shogle::unexpected<std::string>("Error"))
        .transform([&]() {
          value = 8;
          return value;
        });
    REQUIRE_FALSE(err_void_rvalue.has_value());
    REQUIRE(value == 0);
  }

  // TODO: More tests for and_then, or_else and transform_error
  SECTION("and_then") {
    shogle::expected<int, std::string> start(20);

    auto result =
      start.and_then([&](int val) { return safe_divide(val, 2); }).and_then([&](int val) {
        return safe_divide(val, 5);
      });
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 2);

    shogle::expected<int, std::string> start2(20);

    auto result2 =
      start2.and_then([&](int val) { return safe_divide(val, 0); }).and_then([&](int val) {
        return safe_divide(val, 5);
      });

    REQUIRE_FALSE(result2.has_value());
    REQUIRE(result2.error() == "Division by zero");
  }

  SECTION("or_else") {
    shogle::expected<int, std::string> e = shogle::unexpected<std::string>("Fail");

    auto recovered = e.or_else([](const std::string& err) -> shogle::expected<int, std::string> {
      if (err == "Fail") {
        return 0;
      }
      return shogle::unexpected<std::string>(err);
    });

    REQUIRE(recovered.has_value());
    REQUIRE(recovered.value() == 0);
  }

  SECTION("transform_error") {
    shogle::expected<int, int> e = shogle::unexpected<int>(404);

    shogle::expected<int, std::string> res =
      e.transform_error([](int code) { return "Error Code: " + std::to_string(code); });

    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error() == "Error Code: 404");
  }
}
