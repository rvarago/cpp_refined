#include <concepts>
#include <functional>
#include <utility>

#include "error.hpp"

namespace rvarago::refined {

// Properties a refinement implements.
struct traits {
  // Comparison operations with the spaceship operator.
  bool ordered{false};
};

template <typename T, std::predicate<T const &> auto Pred, traits Traits = {},
          typename... Bases>
  requires((std::is_same_v<T, typename Bases::value_type> && ...) &&
           (std::predicate<typename Bases::predicate_type, T const &> && ...))
class refinement {
public:
  using value_type = T;
  using predicate_type = decltype(Pred);

  // `make(value)` is the only factory to refinements.
  //
  // If `Pred(value)` holds, then this produces a valid instance of `T` by
  // delegating to `policy.ok`. Else reports the failure via `policy.err`.
  template <error::policy<refinement> Policy = error::to_optional>
  static constexpr auto make(T value, Policy policy = {})
      -> Policy::template wrapper_type<refinement> {
    if (verify(value) && (Bases::verify(value) && ...)) {
      return policy.template ok<refinement>(
          {unverified_make(std::move(value))});
    } else {
      return policy.template err<refinement>();
    }
  }

  // Accessors to the ground value.
  constexpr auto value() && -> T { return std::move(value_); }
  constexpr auto value() const & -> T const & { return value_; }
  //

  // Implicitly converts to a `Base` (in `Bases...`) refinement.
  //
  // This is safe because we verified `Base`'s predicate upon construction
  // by computing logically-conjuction of all `Bases...`'s predicates.
  template <typename Base>
    requires(std::is_same_v<Base, Bases> || ...)
  constexpr /* implicit */ operator Base() const {
    return Base::unverified_make(value_);
  }

  // `verify(value)` holds when `value` satisfies `Pred`.
  static constexpr auto verify(T const &value) -> bool {
    return std::invoke(Pred, value);
  }

  // `unverified_make(value)` produces a refinement **by-passing** the predicate
  // checking, i.e. with *no* verification whatsover.
  //
  // Use it cautiously, i.e. only when absolutely sure it's fine.
  static constexpr auto unverified_make(T value) -> refinement {
    return refinement{std::move(value)};
  }

  friend constexpr auto operator<=>(refinement const &, refinement const &)
    requires(Traits.ordered)
  = default;

private:
  explicit constexpr refinement(T value) : value_{std::move(value)} {}

  // Ground value.
  T value_;
};

} // namespace rvarago::refined
