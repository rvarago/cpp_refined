#include <catch2/catch_test_macros.hpp>

#include <rvarago/refined.hpp>

namespace refined = rvarago::refined;

using even =
    refined::refinement<int, decltype([](auto const x) { return x % 2 == 0; })>;

TEST_CASE(
    "A refinement type must admit only instance that satisfy its predicate",
    "[make][default_policy]") {

  STATIC_REQUIRE(std::is_same_v<decltype(even::make(0)), std::optional<even>>);

  STATIC_REQUIRE(
      std::is_same_v<decltype(even::make<refined::error::to_optional<even>>(0)),
                     std::optional<even>>);

  STATIC_REQUIRE(even::make(0)->value == 0);
  STATIC_REQUIRE(even::make(1) == std::nullopt);
  STATIC_REQUIRE(even::make(2)->value == 2);
  STATIC_REQUIRE(even::make(3) == std::nullopt);
}

TEST_CASE("A to_exception policy should throw on invalid argument",
          "[error_policy][to_exception]") {

  STATIC_REQUIRE(
      std::is_same_v<
          decltype(even::make<refined::error::to_exception<even>>(10)), even>);

  STATIC_REQUIRE(even::make<refined::error::to_exception<even>>(0).value == 0);
  REQUIRE_THROWS_AS(even::make<refined::error::to_exception<even>>(1),
                    refined::error::to_exception<even>::refinement_exception);
}
