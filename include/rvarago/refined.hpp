#include <concepts>
#include <exception>
#include <functional>
#include <optional>
#include <utility>

namespace rvarago::refined {

// Error policies for the fallible refinements factory.
namespace error {

// Models the error reporting mechanism.
template <typename Policy, typename Refinement>
concept policy = requires(Policy p, Refinement refined) {
  typename Refinement::value_type;
  typename Refinement::predicate_type;

  // On success.
  {
    p.template ok<Refinement>(refined)
  } -> std::same_as<typename Policy::template wrapper_type<Refinement>>;

  // On error.
  {
    p.template err<Refinement>()
  } -> std::same_as<typename Policy::template wrapper_type<Refinement>>;
};

// Report errors as `std::optional<Refinement>`.
struct to_optional {
  template <typename Refinement> using wrapper_type = std::optional<Refinement>;

  // Returns an engaged optional.
  template <typename Refinement>
  constexpr auto ok(Refinement refinement) const -> wrapper_type<Refinement> {
    return std::optional{std::move(refinement)};
  }

  // Returns nullopt.
  template <typename Refinement>
  constexpr auto err() const -> wrapper_type<Refinement> {
    return std::nullopt;
  }
};

// Report errors as exceptions.
struct to_exception {
  template <typename Refinement> using wrapper_type = Refinement;

  struct refinement_exception : std::exception {
    const char *what() const noexcept override {
      return "fail to refine argument due to unsastified predicate";
    }
  };

  // Returns argument unchanged.
  template <typename Refinement>
  constexpr auto ok(Refinement refinement) const -> wrapper_type<Refinement> {
    return refinement;
  }

  // Throws a `refinement_exception`.
  template <typename Refinement>
  constexpr auto err() const -> wrapper_type<Refinement> {
    throw refinement_exception{};
  }
};

} // namespace error

// `refinement<T, Pred>` constraints values `t: T` where `Pred{}(t)` holds.
template <typename T, std::predicate<T const &> Pred> struct refinement {
  using value_type = T;
  using predicate_type = Pred;

  // Base value.
  T value;

  // `make(value, pred)` is the only factory to refinements.
  //
  // If `pred(value)` holds, then produces a valid instance by delegating to
  // `policy.ok`. Else reports error via `policy.err`.
  template <error::policy<refinement<T, Pred>> Policy = error::to_optional>
  static constexpr auto make(T value, Pred pred = {}, Policy policy = {})
      -> Policy::template wrapper_type<refinement<T, Pred>> {
    if (std::invoke(std::move(pred), value)) {
      return policy.template ok<refinement<T, Pred>>(
          {refinement<T, Pred>{std::move(value)}});
    } else {
      return policy.template err<refinement<T, Pred>>();
    }
  }

private:
  explicit constexpr refinement(T val) : value{std::move(val)} {}
};

} // namespace rvarago::refined
