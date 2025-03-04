#include <concepts>
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
  { p.ok(refined) } -> std::same_as<typename Policy::return_type>;

  // On error.
  { p.err() } -> std::same_as<typename Policy::return_type>;
};

// Report errors as `std::optional<Refinement>`.
template <typename Refinement> struct to_optional {
  using return_type = std::optional<Refinement>;

  // Returns an engaged optional.
  constexpr auto ok(Refinement refinement) const -> return_type {
    return std::optional{std::move(refinement)};
  }

  // Returns nullopt.
  constexpr auto err() const -> return_type { return std::nullopt; }
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
  template <error::policy<refinement<T, Pred>> Policy =
                error::to_optional<refinement<T, Pred>>>
  static constexpr auto make(T value, Pred pred = {}, Policy policy = {})
      -> Policy::return_type {
    if (std::invoke(std::move(pred), value)) {
      return policy.ok({refinement<T, Pred>{std::move(value)}});
    } else {
      return policy.err();
    }
  }

private:
  explicit constexpr refinement(T val) : value{std::move(val)} {}
};

} // namespace rvarago::refined
