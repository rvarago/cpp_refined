#include <catch2/catch_test_macros.hpp>

#include <rvarago/refined.hpp>

using namespace rvarago::refined;

TEST_CASE(
    "A refinement type must admit only instance that satisfy its predicate",
    "[make]") {
  using even =
      refinement<int, decltype([](auto const x) { return x % 2 == 0; })>;

  STATIC_REQUIRE(even::make(0)->value == 0);
  STATIC_REQUIRE(even::make(1) == std::nullopt);
  STATIC_REQUIRE(even::make(2)->value == 2);
  STATIC_REQUIRE(even::make(3) == std::nullopt);
}
