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

template <typename T, std::predicate<T const &> auto Pred, typename... Bases>
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
          {refinement::unverified_make(std::move(value))});
    } else {
      return policy.template err<refinement>();
    }
  }

  // Accessors to the ground type.
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

private:
  explicit constexpr refinement(T value) : value_{std::move(value)} {}

  // Ground value.
  T value_;
};

} // namespace rvarago::refined
