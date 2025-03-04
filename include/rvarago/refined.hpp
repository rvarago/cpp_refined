#include <concepts>
#include <functional>
#include <optional>
#include <utility>

namespace rvarago::refined {

// `refinement<T, Pred>` constraints values `t: T` where `Pred{}(t)` holds.
template <typename T, std::predicate<T const &> Pred> struct refinement {
  using value_type = T;
  using predicate_type = Pred;

  // Base value.
  T value;

  // `make(value, pred)` is the only factory to refinements.
  //
  // It returns an engaged optional when `pred(value)` holds.
  static constexpr auto make(T value, Pred pred = {})
      -> std::optional<refinement<T, Pred>> {
    if (std::invoke(std::move(pred), value)) {
      return std::optional{refinement<T, Pred>{std::move(value)}};
    } else {
      return std::nullopt;
    }
  }

private:
  explicit constexpr refinement(T val) : value{std::move(val)} {}
};

} // namespace rvarago::refined
